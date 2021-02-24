#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_gpio.h"
#include "nrf_power.h"
#include "nrf_ppi.h"
#include "nrf_rtc.h"
#include "nrf_saadc.h"
#include "nrf_uart.h"
#include "printf.h"
#include <stdbool.h>
#include <stdint.h>

#include "flync.h"
#include "peripherals.h"

static volatile pt_event_t pofwarn_evt = pt_event_init();

__attribute__((long_call, section(".ramfunctions"))) void
enter_low_power(void) {
  __WFE();
  __SEV();
  __WFE();
}

int uart_init(uint32_t pseltxd) {
  nrf_uart_configure(NRF_UART0, NRF_UART_PARITY_EXCLUDED,
                     NRF_UART_HWFC_DISABLED);
  NRF_UART0->ENABLE = 4UL;
  /* Setup TX pin only */
  NRF_UART0->PSEL.TXD = pseltxd;
  /* Disconnect RX pin */
  NRF_UART0->PSEL.RXD |= (1UL << 31);
  nrf_uart_baudrate_set(NRF_UART0, NRF_UART_BAUDRATE_1000000);
  return 0;
}

void _putchar(char character) {

  NRF_UART0->TXD = character;
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->TASKS_STARTTX = 1UL;
  while (NRF_UART0->EVENTS_TXDRDY == 0) {
  };
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->TASKS_STOPTX = 1UL;
}

int wdt_init(unsigned int seconds) {
  NRF_WDT->CRV = seconds * 32768;
  NRF_WDT->RREN = 0x01;
  NRF_WDT->TASKS_START = 1;
  return 0;
}
__attribute__((long_call, section(".ramfunctions"))) int wdt_reload(void) {
  NRF_WDT->RR[0] = 0x6E524635;
  return 0;
}
void POWER_CLOCK_IRQHandler(void) {
  if (NRF_POWER->EVENTS_POFWARN == 1) {
    NRF_POWER->EVENTS_POFWARN = 0;
    pt_event_set(&pofwarn_evt);
    NRF_POWER->POFCON &= ~(1 << 0);
    NRF_POWER->INTENCLR = POWER_INTENCLR_POFWARN_Msk;
    NVIC_DisableIRQ(POWER_CLOCK_IRQn);
  }
}

__attribute__((long_call, section(".ramfunctions"))) volatile pt_event_t *
pofwarn_request_evt(nrf_power_pof_thr_t pofwarn_thr) {

  pt_event_clear(&pofwarn_evt);
  NRF_POWER->EVENTS_POFWARN = 0;
  NRF_POWER->INTENSET = POWER_INTENSET_POFWARN_Msk;
  NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
  NVIC_EnableIRQ(POWER_CLOCK_IRQn);
  NRF_POWER->POFCON |= (pofwarn_thr << 1) | (1 << 0);
  return &pofwarn_evt;
}

int pofwarn_disable(void) {
  NRF_POWER->POFCON &= ~(1 << 0);
  NRF_POWER->EVENTS_POFWARN = 0;
  NRF_POWER->INTENCLR = POWER_INTENCLR_POFWARN_Msk;
  return 0;
}

volatile int16_t adc_result;

int adc_init() {
  nrf_saadc_channel_config_t saadc_channel_config = {
      .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
      .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
      .gain = NRF_SAADC_GAIN1_6,
      .reference = NRF_SAADC_REFERENCE_INTERNAL,
      .acq_time = NRF_SAADC_ACQTIME_3US,
      .mode = NRF_SAADC_MODE_SINGLE_ENDED,
      .burst = NRF_SAADC_BURST_DISABLED,
      .pin_p = NRF_SAADC_INPUT_VDD,
      .pin_n = NRF_SAADC_INPUT_DISABLED,
  };

  nrf_saadc_channel_init(0, &saadc_channel_config);
  nrf_saadc_buffer_init((nrf_saadc_value_t *)&adc_result, 1);
  nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_10BIT);

  nrf_saadc_enable();

  /* Trigger ADC start on flync clock edge */
  NRF_PPI->CH[1].EEP = (uint32_t)&NRF_RTC0->EVENTS_COMPARE[1];
  NRF_PPI->CH[1].TEP = (uint32_t)&NRF_SAADC->TASKS_START;
  NRF_PPI->CHENSET = PPI_CHENSET_CH1_Msk;

  /* Take sample right after starting ADC */
  NRF_PPI->CH[2].EEP = (uint32_t)&NRF_SAADC->EVENTS_STARTED;
  NRF_PPI->CH[2].TEP = (uint32_t)&NRF_SAADC->TASKS_SAMPLE;
  NRF_PPI->CHENSET = PPI_CHENSET_CH2_Msk;

  /* Immediately stop ADC after conversion is done */
  NRF_PPI->CH[3].EEP = (uint32_t)&NRF_SAADC->EVENTS_DONE;
  NRF_PPI->CH[3].TEP = (uint32_t)&NRF_SAADC->TASKS_STOP;
  NRF_PPI->CHENSET = PPI_CHENSET_CH3_Msk;

  NRF_SAADC->INTENSET = SAADC_INTENSET_END_Msk;
  NVIC_EnableIRQ(SAADC_IRQn);
  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) int
cap_discharge(unsigned int v_level) {
  adc_result = (1 << ADC_BITS);
  NRF_SAADC->INTENCLR = SAADC_INTENCLR_END_Msk;
  while (adc_result > v_level) {
    NRF_SAADC->EVENTS_END = 0;
    NRF_SAADC->TASKS_START = 1;
    while (NRF_SAADC->EVENTS_END == 0) {
      __NOP();
    }
  }
  NRF_SAADC->EVENTS_END = 0;
  NVIC_ClearPendingIRQ(SAADC_IRQn);
  NRF_SAADC->INTENSET = SAADC_INTENSET_END_Msk;

  return 0;
}

unsigned int adc_read_vcap(void) { return adc_result; }

int adc_trigger(void) {
  NVIC_DisableIRQ(SAADC_IRQn);

  /* Allow pending interrupts to wakeup CPU */
  SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
  NRF_SAADC->EVENTS_END = 0;
  NRF_SAADC->TASKS_START = 1;

  while (NRF_SAADC->EVENTS_END == 0) {
    enter_low_power();
  }

  /* Disable pending interrupts waking up CPU */
  SCB->SCR &= ~SCB_SCR_SEVONPEND_Msk;

  NRF_SAADC->EVENTS_END = 0;
  NVIC_ClearPendingIRQ(SAADC_IRQn);
  NVIC_EnableIRQ(SAADC_IRQn);

  return 0;
}

int led_blink(unsigned int led_pin, unsigned int duration_us) {
  unsigned int duration_tcks = duration_us * 16;
  NRF_P0->OUTSET = (1 << led_pin);
  for (unsigned int i = 0; i < duration_tcks / 2; i++) {
    __NOP();
  }
  NRF_P0->OUTCLR = (1 << led_pin);
  return 0;
}

int enable_fpu(void) {
  SCB->CPACR |= (3UL << 20) | (3UL << 22);
  __DSB();
  __ISB();
  return 0;
}

int disable_fpu(void) {
  SCB->CPACR &= ~((3UL << 20) | (3UL << 22));
  return 0;
}