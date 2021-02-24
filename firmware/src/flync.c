#include "flync.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_power.h"
#include "nrf_ppi.h"
#include "nrf_rtc.h"
#include "nrf_saadc.h"
#include <stdbool.h>
#include <stdint.h>

#include "disco.h"
#include "peripherals.h"
#include "printf.h"
#include "prng.h"
#include "timer.h"

#define RTC_FREQUENCY 32768UL
/* 10ms on the 32768Hz clock */
#define FLYNC_CLOCK_RTCTICKS (RTC_FREQUENCY / FLYNC_CLOCK_FREQ_HZ)

/* 1/Kp the proportional controller coefficient */
#define FLYNC_K_P 32
/* 1/Ki the integral controller coefficient */
#define FLYNC_K_I 512

/* RTC0 CC[1] always contains the current flync period estimate in rtc ticks */
#define FLYNC_PERIOD_REG NRF_RTC0->CC[1]

/* Dischargce capacitor to this value to avoid overvoltage shutdown */
#define V_INIT_TRGT 3.0

/* Allows to check if a flync clock edge has been detected between two RTC
 * events */
static volatile bool clock_edge_detected = false;

int flync_setup_gpio(unsigned int gpio_pin);

__attribute__((long_call, section(".ramfunctions"))) static inline int
pi_control(unsigned int phase, unsigned int period) {
  static int err_sum = 0;

  /*
   * We multiply by 128 to keep rounding losses low
   * The setpoint for the phase is half the clock period
   */
  int err = phase * 128 - period * 128 / 2;
  err_sum += err;

  return (err / FLYNC_K_P + err_sum / FLYNC_K_I) / 128;
}

int flync_setup_gpio(unsigned int gpio_pin) {
  NRF_GPIOTE->CONFIG[2] = (GPIOTE_CONFIG_MODE_Task << 0) | (gpio_pin << 8) |
                          (GPIOTE_CONFIG_OUTINIT_Low << 20);

  NRF_PPI->CH[10].EEP = (uint32_t)&NRF_RTC0->EVENTS_COMPARE[1];
  NRF_PPI->CH[10].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[2];

  NRF_PPI->CH[11].EEP = (uint32_t)&NRF_SAADC->EVENTS_END;
  NRF_PPI->CH[11].TEP = (uint32_t)&NRF_GPIOTE->TASKS_CLR[2];

  NRF_PPI->CHENSET = PPI_CHEN_CH10_Msk | PPI_CHEN_CH11_Msk;
  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) unsigned int
flync_phase2rtctcks(unsigned int phase) {
  return (FLYNC_PERIOD_REG + (phase * FLYNC_PERIOD_REG * 2)) / 6;
}

int flync_init(unsigned int gpio_pin) {

  if (gpio_pin < FLYNC_NO_GPIO)
    flync_setup_gpio(gpio_pin);

  /* Discharge capacitor to guarantee current flow and clock signal */
  cap_discharge(V2ADC(V_INIT_TRGT));

#if FLYNC_ACTIVE

  /* Configure GPIOTE IN event on falling edge of clock signal */
  nrf_gpio_cfg_input(FLYNC_PIN_CLK, NRF_GPIO_PIN_NOPULL);
  nrf_gpiote_event_configure(0, FLYNC_PIN_CLK, NRF_GPIOTE_POLARITY_LOTOHI);
  nrf_gpiote_event_enable(0);
  nrf_gpiote_event_clear(0);
  nrf_gpiote_int_enable(NRF_GPIOTE_INT_IN0_MASK);

  /* Allow pending interrupts to wakeup CPU */
  SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

  /* Wait for two rising edges */
  for (unsigned int i = 0; i < 2; i++) {

    NRF_RTC0->TASKS_CLEAR = 1;

    while (NRF_GPIOTE->EVENTS_IN[0] == 0) {
      __WFE();
      __SEV();
      __WFE();
    };
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
  }

  /* Current counter value should be exactly one flync period */
  FLYNC_PERIOD_REG = NRF_RTC0->COUNTER;
  NRF_RTC0->TASKS_CLEAR = 1;

  /* Disable pending interrupts to wakeup CPU */
  SCB->SCR &= ~SCB_SCR_SEVONPEND_Msk;

  /* Disable energy expensive GPIOTE IN event */
  NRF_GPIOTE->INTENCLR = GPIOTE_INTENCLR_IN0_Msk;
  NRF_GPIOTE->CONFIG[0] = 0x0;

  /* Reconfigure GPIOTE for lower power PORT event */
  nrf_gpio_cfg_sense_input(FLYNC_PIN_CLK, NRF_GPIO_PIN_NOPULL,
                           NRF_GPIO_PIN_SENSE_LOW);

  NRF_GPIOTE->EVENTS_PORT = 0;
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
  NVIC_EnableIRQ(GPIOTE_IRQn);

#else
  /* Configure RTC for 1ms interrupt */
  NRF_RTC0->TASKS_CLEAR = 1;
  FLYNC_PERIOD_REG = 32;
#endif
  NRF_PPI->CH[0].EEP = (uint32_t)&NRF_RTC0->EVENTS_COMPARE[1];
  NRF_PPI->CH[0].TEP = (uint32_t)&NRF_RTC0->TASKS_CLEAR;
  NRF_PPI->CHENSET = PPI_CHENSET_CH0_Msk;
  NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE1_Msk;
  NVIC_EnableIRQ(RTC0_IRQn);

  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) void
GPIOTE_IRQHandler(void) {
  /* This handles the GPIO interrupt from the flync circuit. It should trigger
   * on every falling edge of the flync clock signal. We timestamp the
   * on-board RTC timer and run a PI control pased PLL to synchronize the
   * RTC to the external clock signal.
   */
  if (NRF_GPIOTE->EVENTS_PORT == 1) {
    NRF_GPIOTE->EVENTS_PORT = 0;

    /* Check if we've just fired and this was just a spurious edge */
    if (clock_edge_detected) {
      return;
    }

    /* Check if counter has reasonable value, else this may be spurious edge */
    if ((NRF_RTC0->COUNTER > FLYNC_CLOCK_RTCTICKS * 3 / 4) ||
        (NRF_RTC0->COUNTER < FLYNC_CLOCK_RTCTICKS / 4)) {
      return;
    }
    // NRF_P0->OUTSET = (1 << FLYNC_PIN_DBG2);

    /* Calculate clock correction using PI control loop */
    int correction = pi_control(NRF_RTC0->COUNTER, FLYNC_PERIOD_REG);
    /* Update flync period register (and thereby timer period) */
    FLYNC_PERIOD_REG = FLYNC_CLOCK_RTCTICKS + correction;

    clock_edge_detected = true;
    // NRF_P0->OUTCLR = (1 << FLYNC_PIN_DBG2);
  }
}

__attribute__((long_call, section(".ramfunctions"))) void
SAADC_IRQHandler(void) {
/*
 * The SAADC is triggered to read the supply voltage at every rising edge of
 * the PLL-disciplined flync clock. We use this interrupt handler to check
 * for potential MPPT phase and to process the result of the supply voltage
 * reading, potentially informing the application if it crosses a turn-on
 * threshold.
 */
#if FLYNC_ACTIVE
  static unsigned int mppt_countdown = 0;
#endif
  if (NRF_SAADC->EVENTS_END == 1) {
    NRF_SAADC->EVENTS_END = 0;

#if FLYNC_ACTIVE
    /* Are we currently in MPPT phase? */
    if (mppt_countdown) {
      /* Re-enable gpio interrupt, if we've waited for long enough */
      if (--mppt_countdown == 0) {
        NVIC_ClearPendingIRQ(GPIOTE_IRQn);
        NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
      }
    }
    /* Assume that we're in MPPT phase, when there's been no gpio interrupt */
    else if (!clock_edge_detected) {
      /* Disable the GPIOTE interrupt for MPPT phase */
      NRF_GPIOTE->EVENTS_PORT = 0;
      NRF_GPIOTE->INTENCLR = GPIOTE_INTENCLR_PORT_Msk;
      mppt_countdown = 28;
    }
    clock_edge_detected = false;
#endif

    /* Update system timer */
    _timer_tick();
  }
}
