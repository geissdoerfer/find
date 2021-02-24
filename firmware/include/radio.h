#ifndef __RADIO_H_
#define __RADIO_H_

#include <stdbool.h>
#include <stdint.h>

#define RADIO_NO_GPIO 32

enum RadioLogicalAddress {
  /* Heading beacon logical address */
  LA_BCN = 1,
  /* Trailing beacon, no detection */
  LA_ACK_NO = 2,
  /* Trailing beacon, successful detection */
  LA_ACK_YES = 3
};

/**
 * Initializes radio peripheral
 *
 * @param gpio_pin Pin used for debug. Provide RADIO_NO_GPIO to disable.
 *
 * @returns 0 on success
 */
int radio_init(unsigned int gpio_pin);

/**
 * Enables radio event
 *
 * Enables event and interrupt for given hardware event.
 *
 * @param hw_event Register address of hardware event
 * @param intenset_mask Mask of corresponding interrupt
 *
 * @returns 0 on success
 */
int radio_enable_event(volatile uint32_t *hw_event, uint32_t intenset_mask);

/**
 * Shutdown radio
 *
 * Disables radio, allowing device to resume low power operation.
 *
 * @returns 0 on success
 */
int radio_shutdown();

#endif /* __RADIO_H_ */