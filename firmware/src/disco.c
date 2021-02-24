#include <string.h>

#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_radio.h"
#include "nrf_rtc.h"
#include "nrf_saadc.h"

#include "disco.h"
#include "flync.h"
#include "peripherals.h"
#include "printf.h"
#include "prng.h"
#include "timer.h"

static disco_queue_t disco_queue = pt_queue_init();

static volatile pt_event_t radio_evt = pt_event_init();

/* Payload of transmitted beacon/acknowledgement */
static disco_pdu_t tx_pdu;

/* Buffer for received beacon/acknowledgment payload */
static disco_pdu_t rx_pdu;

__attribute__((long_call, section(".ramfunctions"))) int
disco_strfmt(char *buf, disco_data_t *disco_data, unsigned int t_chr,
             size_t buf_size) {
  int count =
      snprintf(buf, buf_size, "%08X,%u\n", disco_data->device_id, t_chr);
  return count;
}

int disco_init(unsigned int gpio_pin) {
  tx_pdu.device_id = NRF_FICR->DEVICEADDR[0];
  return radio_init(gpio_pin);
}

__attribute__((long_call, section(".ramfunctions"))) disco_data_t *
disco_results_pop(void) {
  return pt_queue_pop(&disco_queue);
}

static inline int disco_prepare_beacon(uint16_t t_chr,
                                       unsigned int start_time_tcks) {

  /* Start HFCLK */
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  /* Set timer to start beacon transmission */
  NRF_RTC0->CC[0] = start_time_tcks + 20;
  NRF_PPI->CHENSET = PPI_CHENSET_CH28_Msk;
  NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;

  /* Setup payload and txaddress for beacon */
  NRF_RADIO->TXADDRESS = LA_BCN;

  tx_pdu.t_chr = t_chr;
  NRF_RADIO->PACKETPTR = (uint32_t)&tx_pdu;

  /* Disable PPI channel that switches off HFCLK when radio is disabled */
  NRF_PPI->CHEN &= ~PPI_CHEN_CH4_Msk;

  /* Prepare radio to listen after sending the beacon */
  NRF_RADIO->SHORTS &= ~NRF_RADIO_SHORT_DISABLED_TXEN_MASK;
  NRF_RADIO->SHORTS |=
      NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_DISABLED_RXEN_MASK;

  pt_event_clear(&radio_evt);
  radio_enable_event(&(NRF_RADIO->EVENTS_TXREADY), RADIO_INTENSET_TXREADY_Msk);

  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) static inline int
disco_prepare_rx() {

  /* Disable link RTC->TXEN */
  NRF_PPI->CHENCLR = PPI_CHENCLR_CH28_Msk;
  NRF_RTC0->EVTENCLR = RTC_EVTENSET_COMPARE0_Msk;

  /* Disable shorts so radio stays in RX */
  NRF_RADIO->SHORTS &=
      ~(NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_DISABLED_RXEN_MASK);

  NRF_RADIO->SHORTS |= NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK;
  radio_enable_event(&(NRF_RADIO->EVENTS_END), RADIO_INTENSET_END_Msk);

  pt_event_clear(&radio_evt);

  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) static inline int
disco_send_ack() {

  /* Stop RX and turnaround for transmitting ACK */
  NRF_RADIO->SHORTS &= ~NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK;
  NRF_RADIO->SHORTS |=
      (NRF_RADIO_SHORT_END_DISABLE_MASK | NRF_RADIO_SHORT_DISABLED_TXEN_MASK);
  NRF_RADIO->PACKETPTR = (uint32_t)&tx_pdu;
  NRF_RADIO->TXADDRESS =
      (pt_queue_empty(&disco_queue)) ? LA_ACK_NO : LA_ACK_YES;

  NRF_RADIO->TASKS_DISABLE = 1;

  pt_event_clear(&radio_evt);
  radio_enable_event(&(NRF_RADIO->EVENTS_TXREADY), RADIO_INTENSET_TXREADY_Msk);

  return 0;
}

__attribute__((long_call, section(".ramfunctions"))) static inline int
disco_handle_pkt() {

  /* Go back to RX if packet is damaged */
  if (NRF_RADIO->CRCSTATUS == 0) {
    return -1;
  }
  radio_enable_event(&(NRF_RADIO->EVENTS_END), RADIO_INTENSET_END_Msk);

  disco_data_t disco_data;
  disco_data.device_id = rx_pdu.device_id;
  disco_data.rssi = NRF_RADIO->RSSISAMPLE;
  if ((NRF_RADIO->RXMATCH == LA_BCN) || (NRF_RADIO->RXMATCH == LA_ACK_YES) ||
      (NRF_RADIO->RXMATCH == LA_ACK_NO)) {
    disco_data.type = NRF_RADIO->RXMATCH;
    pt_queue_push(&disco_queue, disco_data);
    return 0;
  }
  return -1;
}

__attribute__((long_call, section(".ramfunctions"))) void
disco_thread(struct pt *pt, uint16_t t_chr, unsigned int rx_wdw_tcks,
             unsigned int start_time_tcks) {
  pt_begin(pt);

  static volatile pt_event_t *rtc_event;

  if (start_time_tcks > 21) {
    /* Wait for timer before starting HFCLK */
    rtc_event = timer_rtc_set(start_time_tcks - 20);
    pt_event_wait(pt, rtc_event);
  }

  /* Prepare and send beacon, once HFCLK is up */
  disco_prepare_beacon(t_chr, start_time_tcks);

  /* Wait until radio has ramped up for TX */
  pt_event_wait(pt, &radio_evt);

  /* Switch to RX buffer */
  NRF_RADIO->PACKETPTR = (uint32_t)&rx_pdu;

  /* Wait for radio to ramp up to RX */
  pt_event_clear(&radio_evt);
  radio_enable_event(&(NRF_RADIO->EVENTS_RXREADY), RADIO_INTENSET_RXREADY_Msk);
  pt_event_wait(pt, &radio_evt);

  disco_prepare_rx();

  unsigned int timeout = NRF_RTC0->COUNTER + rx_wdw_tcks;
  if (timeout > NRF_RTC0->CC[1])
    rtc_event = timer_rtc_set(timeout - NRF_RTC0->CC[1]);
  else
    rtc_event = timer_rtc_set(timeout);

  /* Listen until rx window timer expires */
  while (!pt_event_get(rtc_event)) {
    enter_low_power();
    /* We've received an address */
    if (pt_event_get(&radio_evt)) {
      pt_event_clear(&radio_evt);
      disco_handle_pkt();
    }
  }
  /* Send trailing beacon */
  disco_send_ack();
  /* Wait until radio has ramped up for TX */
  pt_event_wait(pt, &radio_evt);
  /* Prepare radio to shut down after sending beacon */
  radio_shutdown();
  pt_end(pt);
}

__attribute__((long_call, section(".ramfunctions"))) void RADIO_IRQHandler() {
  if ((NRF_RADIO->EVENTS_TXREADY == 1) &&
      (NRF_RADIO->INTENSET & RADIO_INTENSET_TXREADY_Msk)) {
    NRF_RADIO->EVENTS_TXREADY = 0;
    pt_event_set(&radio_evt);
    NRF_RADIO->INTENCLR = RADIO_INTENCLR_TXREADY_Msk;
  }
  if ((NRF_RADIO->EVENTS_RXREADY == 1) &&
      (NRF_RADIO->INTENSET & RADIO_INTENSET_RXREADY_Msk)) {
    NRF_RADIO->EVENTS_RXREADY = 0;
    pt_event_set(&radio_evt);
    NRF_RADIO->INTENCLR = RADIO_INTENCLR_RXREADY_Msk;
  }
  if ((NRF_RADIO->EVENTS_END == 1) &&
      (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk)) {
    NRF_RADIO->EVENTS_END = 0;
    pt_event_set(&radio_evt);
    NRF_RADIO->INTENCLR = RADIO_INTENCLR_END_Msk;
  }
}
