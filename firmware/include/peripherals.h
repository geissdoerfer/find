#ifndef __PERIPHERALS_H__
#define __PERIPHERALS_H__

#include "pt.h"
#include <nrf_power.h>
#include <stdint.h>

#define ADC_GAIN 1 / 6
#define ADC_REF 0.6
#define ADC_BITS 10

#define V2ADC(x) (unsigned int)(x * ADC_GAIN / ADC_REF * (1 << ADC_BITS))

extern volatile int16_t adc_result;
/**
 * Puts CPU to sleep
 *
 * CPU enters sleep and only wakes up on pending interrupt.
 *
 */
void enter_low_power(void);

/**
 * Initializes UART peripheral
 *
 * Sets up UART for blocking TX at fixed baudrate.
 *
 * @param pseltxd TX pin
 *
 * @returns 0 on success
 */
int uart_init(uint32_t pseltxd);

/**
 * Transmits character over UART
 *
 * Standalone or glue function for printf implementation.
 *
 * @param character Character to transmit
 *
 */
void _putchar(char character);

/**
 * Sets up powerfail comparator
 *
 * Configures POFCON peripheral for given threshold. The comparator only works
 * while the CPU is active! Event is set as soon as supply voltage drops below
 * given threshold.
 *
 * @param pofwarn_thr Threshold at which event is set
 *
 * @returns Event to be set
 */
volatile pt_event_t *pofwarn_request_evt(nrf_power_pof_thr_t pofwarn_thr);

/**
 * Disables powerfail comparator
 *
 * @returns 0 on success
 */
int pofwarn_disable(void);

/**
 * Initializes watchdog timer
 *
 * Sets up watchdog for given period. If watchdog is not reloaded within given
 * period, it resets the system.
 *
 * @param seconds Number of seconds before watchdog reset
 *
 * @returns 0 on success
 */
int wdt_init(unsigned int seconds);

/**
 * Reloads watchdog timer
 *
 * Has to be called periodically, while watchdog is active to prevent system
 * reset.
 *
 * @returns 0 on success
 */
int wdt_reload(void);

/**
 * Initializes ADC for supply voltage reading
 *
 * Sets up ADC peripheral to take a sample of the supply (capacitor) voltage
 * with every compare match of the RTC peripheral that is synchronized to the
 * external flync clock signal. Configures ADC to fire interrupt when result
 * is available. This interrupt plays a crucial role in the flync core
 * implementation.
 *
 * @returns 0 on success
 */
int adc_init(void);

/**
 * Reads last result of ADC sampling
 *
 * Simple getter method for buffer where ADC stores samples of supply voltage.
 * Note that this does not trigger a new sample, so value may be outdated. Use
 * `adc_trigger` to trigger sample and guarantee up to date value.
 *
 * @returns Supply voltage reading in binary ADC code
 */
unsigned int adc_read_vcap(void);

/**
 * Triggers ADC sample
 *
 * Triggers ADC sample and blocks until sample has been stored in ADC buffer.
 *
 * @returns 0 on success
 */
int adc_trigger(void);

/**
 * Discharges capacitor to given level
 *
 * Having the capacitor fully charged can be a problem for the flync clock.
 * This function continuously samples the supply voltage and runs the CPU
 * until the voltage drops below the given threshold.
 *
 * @param v_level Voltage to which capacitor should be discharged
 *
 * @returns 0 on success
 */
int cap_discharge(unsigned int v_level);

/**
 * Blinks LED
 *
 * Switches on provided LED pin for given duration. Works with active high
 * LED and assumes that GPIO has been configured as output. Duration is not
 * guaranteed, but a very coarse approximation.
 *
 * @param led_pin GPIO pin number that LED is connected to
 * @param duration_us Duration for which LED should be on in microseconds
 *
 * @returns 0 on success
 */
int led_blink(unsigned int led_pin, unsigned int duration_us);

/**
 * Enables FPU
 *
 * @returns 0 on success
 */
int enable_fpu(void);

/**
 * Disables FPU
 *
 * @returns 0 on success
 */
int disable_fpu(void);

#endif /* __PERIPHERALS_H__ */