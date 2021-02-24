#ifndef __TIMER_H_
#define __TIMER_H_
#include "pt.h"

/**
 * Current system time in flync ticks
 *
 * System runs a 32-bit timer counting flync clock periods. The timer is
 * incremented on every rising edge of the clock signal.
 *
 * @returns Current system time in flync ticks
 */
unsigned int timer_now(void);

/**
 * Sets an RTC timer by wait time
 *
 * Allows to setup a timer with respect to the PLL-disciplined 32kHz RTC
 * timer. The timer is reset on every rising edge of the flync clock. Timer
 * is fired only once.
 *
 * @param wait_ticks Number of RTC ticks before event is triggered
 *
 * @returns Event that is set after specified wait time
 */
volatile pt_event_t *timer_rtc_wait(unsigned int wait_ticks);
/**
 * Sets an RTC timer
 *
 * Allows to setup a timer with respect to the PLL-disciplined 32kHz RTC
 * timer. The timer is reset on every rising edge of the flync clock. Timer
 * is fired only once.
 *
 * @param rtc_ticks Timer timeout in RTC ticks
 *
 * @returns Event that is set at specified time on RTC
 */
volatile pt_event_t *timer_rtc_set(unsigned int rtc_ticks);

/**
 * Cancels RTC timer
 *
 * Cancels a previously set RTC timer so that it can be set again.
 *
 */
int timer_rtc_cancel(void);

/**
 * Sets a flync timer by wait time
 *
 * Allows to setup a timer with respect to the flync clock. Timer is set to
 * fire after specified number of flync clock edges. Setting wait_time to zero
 * triggers event at next clock edge.
 *
 * @param wait_time Number of clock edges to wait before event is triggered
 *
 * @returns Event that is set after specified wait time
 */
volatile pt_event_t *timer_flync_wait(unsigned int wait_time);

/**
 * Sets a flync timer by absolute time
 *
 * Allows to setup a timer with respect to the flync clock. Timer is set to
 * fire on the rising edge when the system time reaches the specified time.
 *
 * @param wait_time Time at which event should be triggered
 *
 * @returns Event that is set at specified time
 */
volatile pt_event_t *timer_flync_set(unsigned int time);

/**
 * Cancels flync timer
 *
 * Cancels a previously set flync timer so that it can be set again.
 *
 */
int timer_flync_cancel(void);

/**
 * Increments the system time
 *
 * Must be called externally at every flync clock edge. Increments the
 * internal counter and checks if the flync timer has expired.
 *
 */
void _timer_tick(void);

#endif /* __TIMER_H_ */