#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_rtc.h"
#include "pt.h"
#include <stdbool.h>

#include "printf.h"
#include "timer.h"

#define TIMER_INACTIVE 0

/* This holds the current system time in flync clock ticks */
static unsigned int timer_counter = 1;

/* Event for RTC timer */
static volatile pt_event_t rtc_event = pt_event_init();
/* Event for flync timer */
static volatile pt_event_t timer_event = pt_event_init();

/* Stores value of flync timer */
static unsigned int timer_timeout = 0;

__attribute__((long_call, section(".ramfunctions"))) unsigned int
timer_now(void) {
  return timer_counter;
}

__attribute__((long_call, section(".ramfunctions"))) volatile pt_event_t *
timer_rtc_wait(unsigned int wait_ticks) {
  return timer_rtc_set(NRF_RTC0->COUNTER + wait_ticks);
}

__attribute__((long_call, section(".ramfunctions"))) volatile pt_event_t *
timer_rtc_set(unsigned int rtc_ticks) {

  /* Make sure that the timer is not already armed */
  if (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE1_Msk)
    timer_rtc_cancel();

  NRF_RTC0->CC[0] = rtc_ticks;
  NRF_RTC0->EVENTS_COMPARE[0] = 0;
  NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;
  pt_event_clear(&rtc_event);
  return &rtc_event;
}

__attribute__((long_call, section(".ramfunctions"))) int
timer_rtc_cancel(void) {
  if ((NRF_RTC0->EVTEN & RTC_EVTEN_COMPARE0_Msk) == 0)
    return -1;

  NRF_RTC0->EVTENCLR = RTC_EVTENCLR_COMPARE0_Msk;
  NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
  NRF_RTC0->EVENTS_COMPARE[0] = 0;

  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) volatile pt_event_t *
timer_flync_wait(unsigned int wait_time) {
  return timer_flync_set(timer_counter + 1 + wait_time);
}

__attribute__((long_call, section(".ramfunctions"))) volatile pt_event_t *
timer_flync_set(unsigned int time) {
  /* Make sure timer is not already armed */
  if (timer_timeout != TIMER_INACTIVE)
    timer_flync_cancel();

  /* Cannot set timer in the past */
  if (time < (timer_counter + 1))
    return NULL;
  /* Timer requested at very next clock edge */
  else if (time == (timer_counter + 1)) {
    /* Enable interrupt now */
    NRF_RTC0->EVENTS_COMPARE[1] = 0;
    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE1_Msk;
  }
  timer_timeout = time;
  pt_event_clear(&timer_event);
  return &timer_event;
}

__attribute__((long_call, section(".ramfunctions"))) int
timer_flync_cancel(void) {
  if (timer_timeout == TIMER_INACTIVE)
    return -1;

  NRF_RTC0->EVENTS_COMPARE[1] = 0;
  NVIC_ClearPendingIRQ(RTC0_IRQn);
  NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE1_Msk;

  timer_timeout = TIMER_INACTIVE;
  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) void _timer_tick(void) {
  /* Increment system time */
  timer_counter++;
  /* Check if timer has expired */
  if (timer_timeout == (timer_counter + 1)) {
    NRF_RTC0->EVENTS_COMPARE[1] = 0;
    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE1_Msk;
  }
}

__attribute__((long_call, section(".ramfunctions"))) void
RTC0_IRQHandler(void) {
  /* Interrupt for flync timer */
  if ((NRF_RTC0->EVENTS_COMPARE[1] == 1) &&
      (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE1_Msk)) {
    NRF_RTC0->EVENTS_COMPARE[1] = 0;
    NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE1_Msk;

    pt_event_set(&timer_event);
    timer_timeout = TIMER_INACTIVE;
  }
  /* Interrupt for RTC timer */
  if ((NRF_RTC0->EVENTS_COMPARE[0] == 1) &&
      (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk)) {
    NRF_RTC0->EVENTS_COMPARE[0] = 0;
    NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
    NRF_RTC0->EVTENCLR = RTC_EVTENCLR_COMPARE0_Msk;

    pt_event_set(&rtc_event);
  }
}
