#ifndef __DISCO_H_
#define __DISCO_H_

#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_power.h"

#include "pt.h"
#include "radio.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  /* ID of discovered node */
  uint32_t device_id;
  /* RSSI of packet from discovered node */
  uint8_t rssi;
  /* Type of beacon */
  enum RadioLogicalAddress type;
} disco_data_t;

typedef struct __attribute__((packed)) {
  /* Sender device id */
  uint32_t device_id;
  /* Current charging time */
  uint16_t t_chr;
} disco_pdu_t;

typedef pt_queue(disco_data_t, 16) disco_queue_t;
/**
 * Retrieves result of previous discovery.
 *
 * Discovery runs in protothread and can't return result. Therefore, user has
 * to actively query the result after thread ends.
 *
 * @returns Result of previous discovery round.
 *
 */
disco_data_t *disco_results_pop(void);

/**
 * Initializes discovery protocol and radio
 *
 * @param gpio_pin Pin used for debug. Provide RADIO_NO_GPIO to disable.
 *
 * @returns 0 on success
 */
int disco_init(unsigned int gpio_pin);

/**
 * Executes discovery as protothread.
 *
 * Schedules to start HFCLK and then send a beacon at given phase. After
 * sending the beacon, radio is turned around to listen for acknowledgement
 * of beacon from another node. Stops RX if packet is received, voltage drops
 * below fixed threshold, or after given number of RTC ticks. Result is stored
 * internally and must be retrieved by the user.
 *
 * @param pt Reference to the thread struct managing this thread
 * @param rx_wdw_tcks Length of RX window in RTC ticks
 * @param start_time_tcks Time at which beacon should be sent
 *
 */
void disco_thread(struct pt *pt, uint16_t t_chr, unsigned int rx_wdw_tcks,
                  unsigned int start_time_tcks);

/**
 * Formats a string with results from discovery.
 *
 * @param buf Buffer where formatted string should be stored
 * @param disco_data Discovery data
 * @param t_chr Current charging time
 * @param buf_size Length of buffer
 *
 */
int disco_strfmt(char *buf, disco_data_t *disco_data, unsigned int t_chr,
                 size_t buf_size);

/**
 * Convenience wrapper for running discovery.
 *
 * Protothreads cannot block in nested functions. This macro provides a
 * convenient way to run the discovery protothread from within the context of
 * a parent thread, blocking the parent thread, while discovery thread is
 * blocking.
 *
 * @param pt Handle of parent protothread
 * @param pt_disco Handle of discovery protothread
 * @param params Parameters for discovery
 *
 */
#define pt_disco(pt, pt_disco, t_chr, rx_wdw_tcks, start_time_tcks)            \
  *pt_disco = (const struct pt){0};                                            \
  pt_loop(pt, pt_status(pt_disco) == PT_STATUS_BLOCKED) {                      \
    disco_thread(pt_disco, t_chr, rx_wdw_tcks, start_time_tcks);               \
  }

#endif /* __DISCO_H_ */