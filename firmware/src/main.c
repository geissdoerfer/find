#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_power.h"
#include "nrf_radio.h"
#include "nrf_saadc.h"
#include "nrf_uart.h"

#include "flync.h"
#include "printf.h"

/* Minimum voltage for becoming active */
#define V_THR_ON 3.3
/* Power off threshold */
#define V_THR_OFF NRF_POWER_POFTHR_V28

/* Maximum voltage before converter goes into overvoltage protection mode */
#define V_THR_MAX 3.5

/**
 * Protocol protothread
 *
 * Implements the key protocol handling the three phases, i.e. charging,
 * waiting and discovery. Yields while charging and waiting, but blocks during
 * actual discovery period.
 *
 * @param pt Handle of protothread
 *
 */
__attribute__((long_call, section(".ramfunctions"))) void
protocol(struct pt *pt) {
  pt_begin(pt);

  static struct pt pt_disco_h;
  static volatile pt_event_t *clk_evt;
  static volatile pt_event_t *pof_evt;

  static disco_data_t *disco_data;

  static unsigned int t_start;
  static unsigned int t_charge;

  for (;;) {
    /* Timestamp beginning of charging period */
    t_start = timer_now();

    /* Make sure next reading from ADC is recent */
    pt_yield(pt);

    /* Wait until voltage reaches turn-on threshold */
    pt_wait(pt, adc_read_vcap() > V2ADC(V_THR_ON));
    t_charge = timer_now() - t_start;

    /* Maximum waiting time equals charging time*/
    unsigned int wait_time = geometric_itf_sample(lookup_scale(t_charge));

    /* Wait for waiting time or until capacitor is fully charged */
    clk_evt = timer_flync_wait(wait_time);
    pt_wait(pt, pt_event_get(clk_evt) || (adc_read_vcap() > V2ADC(V_THR_MAX)));

#if FLYNC_ACTIVE
    pt_disco(pt, &pt_disco_h, (uint16_t)t_charge, 26, flync_phase2rtctcks(1));
#else
    pt_disco(pt, &pt_disco_h, (uint16_t)t_charge, 26, 0);
#endif
    pof_evt = pofwarn_request_evt(V_THR_OFF);

    while ((disco_data = disco_results_pop()) != NULL) {
      /* Switch on LED */
      NRF_P0->OUTSET = (1 << FLYNC_LED);
    }

    while (!pt_event_get(pof_evt)) {
      __NOP();
    }
    /* Switch off LED */
    NRF_P0->OUTCLR = (1 << FLYNC_LED);

    wdt_reload();
  }
  pt_end(pt);
}

/**
 * Protothread loop
 *
 * Iterates all threads and enters low power mode when all blocking.
 *
 */
__attribute__((long_call, section(".ramfunctions"))) void thread_loop(void) {
  struct pt pt_protocol = pt_init();
  while (1) {
    protocol(&pt_protocol);
    enter_low_power();
  };
}

int main(void) {

  enable_fpu();

  nrf_gpio_cfg_output(FLYNC_LED);

  nrf_gpio_pin_clear(FLYNC_LED);

  // uart_init(FLYNC_PIN_DBG1);

  /* DC/DC reduces power consumption at high current draw */
  nrf_power_dcdcen_set(true);
  adc_init();
  prng_seed();

  /* Use device address as beacon/ack payload */
  disco_init(RADIO_NO_GPIO);
  flync_init(FLYNC_PIN_DBG2);

  /* 5 seconds watch dog */
  wdt_init(5);

  thread_loop();
}
