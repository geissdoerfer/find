#ifndef __FLYNC_H_
#define __FLYNC_H_

#include <stdint.h>

#include "peripherals.h"
#include "prng.h"
#include "pt.h"
#include "timer.h"
#include "disco.h"
#include "radio.h"

#define FLYNC_ACTIVE 1

/* Used to disable debug GPIO pin */
#define FLYNC_NO_GPIO 32

/* Pin to which the clock signal is connected */
#define FLYNC_PIN_CLK 2

/* Debug pins for Logic Analyzer */
#define FLYNC_PIN_DBG1 23
#define FLYNC_PIN_DBG2 24
#define FLYNC_PIN_DBG3 19

#define FLYNC_LED 25

#define FLYNC_CLOCK_FREQ_HZ 100UL

/**
 * Initializes FLYNC clock
 *
 * @param gpio_pin GPIO pin for debug. Provide FLYNC_NO_GPIO to disable.
 *
 * @returns 0 on success
 */
int flync_init(unsigned int gpio_pin);

/**
 * Converts powerline phase to rtc ticks
 *
 * Calculates value of RTC clock for any of the three phases, i.e. phase 1
 * is at half the PLL disciplined RTC period, phase 0 is at 1/6 and phase 3
 * is at 5/6.
 *
 * @param phase Phase index {0, 1, 2}
 *
 * @returns Phase in RTC ticks
 */
unsigned int flync_phase2rtctcks(unsigned int phase);

#endif /* __FLYNC_H_ */