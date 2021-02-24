#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_gpio.h"
#include "nrf_radio.h"

#include "flync.h"
#include "radio.h"

static int radio_setup_gpio(unsigned int gpio_pin) {
  NRF_GPIOTE->CONFIG[1] = (GPIOTE_CONFIG_MODE_Task << 0) | (gpio_pin << 8) |
                          (GPIOTE_CONFIG_POLARITY_Toggle << 16) |
                          (GPIOTE_CONFIG_OUTINIT_Low << 20);

  NRF_PPI->CH[8].EEP = (uint32_t)&NRF_RADIO->EVENTS_READY;
  NRF_PPI->CH[8].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[1];

  NRF_PPI->CH[9].EEP = (uint32_t)&NRF_RADIO->EVENTS_DISABLED;
  NRF_PPI->CH[9].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[1];

  NRF_PPI->CHENSET = PPI_CHEN_CH8_Msk | PPI_CHEN_CH9_Msk;
  return 0;
}

int radio_shutdown() {

  /* Do not power up radio again after disabling */
  NRF_RADIO->SHORTS &= ~(NRF_RADIO_SHORT_DISABLED_TXEN_MASK |
                         NRF_RADIO_SHORT_DISABLED_RXEN_MASK);

  NRF_RADIO->SHORTS |= NRF_RADIO_SHORT_END_DISABLE_MASK;

  /* Use PPI to power off HFCLK once radio is disabled */
  NRF_PPI->CHEN |= PPI_CHEN_CH4_Msk;

  /* Disable all interrupts */
  NRF_RADIO->INTENCLR = 0xFFFFFFFF;

  return 0;
}

int radio_enable_event(volatile uint32_t *hw_event, uint32_t intenset_mask) {
  *hw_event = 0;
  NRF_RADIO->INTENCLR = 0xFFFFFFFF;
  NRF_RADIO->INTENSET = intenset_mask;
  return 0;
}

int radio_init(unsigned int gpio_pin) {
  /* 0dBm TX power */
  NRF_RADIO->TXPOWER =
      (RADIO_TXPOWER_TXPOWER_Neg8dBm << RADIO_TXPOWER_TXPOWER_Pos);
  /* 2450 MHz frequency */
  NRF_RADIO->FREQUENCY = 50UL;
  /* BLE 1MBit */
  NRF_RADIO->MODE = (RADIO_MODE_MODE_Ble_2Mbit << RADIO_MODE_MODE_Pos);
  /* Fast radio rampup */
  NRF_RADIO->MODECNF0 = (RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos);

  /* We'll only use base address 1, i.e. logical addresses 1-7 */
  NRF_RADIO->BASE1 = 0x12345678;
  /* AP1 for heading beacon (BE),
   * AP2 for trailing beacon with successful detection
   * AP3 for trailing beacon without successful detection
   */
  NRF_RADIO->PREFIX0 = (0xA1 << 24) | (0xA0 << 16) | (0xBE << 8);
  /* We want to receive on both, beacon and ACK addresses */
  NRF_RADIO->RXADDRESSES =
      (1UL << LA_ACK_YES) | (1UL << LA_ACK_NO) | (1UL << LA_BCN);

  /* Make interframe spacing slightly longer than turnaround time (~40us) */
  NRF_RADIO->TIFS = 50;

  /* No S0, LEN and S1 fields */
  NRF_RADIO->PCNF0 = (0 << RADIO_PCNF0_S1LEN_Pos) |
                     (0 << RADIO_PCNF0_S0LEN_Pos) |
                     (0 << RADIO_PCNF0_LFLEN_Pos);

  /* No whitening, little endian, 2B base address, 4B payload */
  NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                     (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
                     (2 << RADIO_PCNF1_BALEN_Pos) |
                     (6 << RADIO_PCNF1_STATLEN_Pos) |
                     (6 << RADIO_PCNF1_MAXLEN_Pos);

  /* One byte CRC */
  NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos);
  NRF_RADIO->CRCINIT = 0xFFUL;
  NRF_RADIO->CRCPOLY = 0x107UL;

  /* Set default shorts */
  NRF_RADIO->SHORTS = NRF_RADIO_SHORT_READY_START_MASK;

  /* Allow to automatically turn off HFCLK after radio is disabled */
  NRF_PPI->CH[4].EEP = (uint32_t)&NRF_RADIO->EVENTS_DISABLED;
  NRF_PPI->CH[4].TEP = (uint32_t)&NRF_CLOCK->TASKS_HFCLKSTOP;

  NVIC_EnableIRQ(RADIO_IRQn);

  if (gpio_pin < RADIO_NO_GPIO)
    radio_setup_gpio(gpio_pin);

  return 0;
}