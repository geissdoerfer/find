#include "nrf52840.h"
#include "nrf52840_bitfields.h"

#define STACK_TOP (void *)0x20002000

extern unsigned long _stext;
extern unsigned long _sbss;
extern unsigned long _sdata;
extern unsigned long _etext;
extern unsigned long _ebss;
extern unsigned long _edata;
extern unsigned long _isr_vector_start;
extern unsigned long _isr_vector_end;

volatile unsigned long ram_vector_table[64] __attribute__((aligned(128)));

/* Exceptions */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void POWER_CLOCK_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void RADIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UARTE0_UART0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void NFCT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOTE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SAADC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TEMP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ECB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CCM_AAR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WDT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void QDEC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void COMP_LPCOMP_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SWI0_EGU0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWI1_EGU1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWI2_EGU2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWI3_EGU3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWI4_EGU4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWI5_EGU5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PWM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PDM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void MWU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PWM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PWM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPIM2_SPIS2_SPI2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void RTC2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2S_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UARTE1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void QSPI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYPTOCELL_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void PWM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

void c_startup(void);
void dummy_fn(void);

int main();

__attribute__((section(".isr_vector"))) void (*vectors[])(void) = {
    STACK_TOP,
    c_startup,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0UL,
    0UL,
    0UL,
    0UL,
    SVC_Handler,
    DebugMon_Handler,
    0UL,
    PendSV_Handler,
    SysTick_Handler,
    POWER_CLOCK_IRQHandler,
    RADIO_IRQHandler,
    UARTE0_UART0_IRQHandler,
    SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler,
    SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler,
    NFCT_IRQHandler,
    GPIOTE_IRQHandler,
    SAADC_IRQHandler,
    TIMER0_IRQHandler,
    TIMER1_IRQHandler,
    TIMER2_IRQHandler,
    RTC0_IRQHandler,
    TEMP_IRQHandler,
    RNG_IRQHandler,
    ECB_IRQHandler,
    CCM_AAR_IRQHandler,
    WDT_IRQHandler,
    RTC1_IRQHandler,
    QDEC_IRQHandler,
    COMP_LPCOMP_IRQHandler,
    SWI0_EGU0_IRQHandler,
    SWI1_EGU1_IRQHandler,
    SWI2_EGU2_IRQHandler,
    SWI3_EGU3_IRQHandler,
    SWI4_EGU4_IRQHandler,
    SWI5_EGU5_IRQHandler,
    MWU_IRQHandler,
    TIMER3_IRQHandler,
    TIMER4_IRQHandler,
    PWM0_IRQHandler,
    PDM_IRQHandler,
    0,
    0,
    PWM1_IRQHandler,
    PWM2_IRQHandler,
    SPIM2_SPIS2_SPI2_IRQHandler,
    RTC2_IRQHandler,
    I2S_IRQHandler,
    FPU_IRQHandler,
    USBD_IRQHandler,
    UARTE1_IRQHandler,
    QSPI_IRQHandler,
    CRYPTOCELL_IRQHandler,
    0,
    0,
    PWM3_IRQHandler,
    0,
    SPIM3_IRQHandler,

};

void Default_Handler(void) {

  NRF_P0->PIN_CNF[25] = 0x03;
  while (1) {
    NRF_P0->OUTSET = (1 << 25);
    for (unsigned int i = 0; i < 100000; i++) {
      __NOP();
    }
    NRF_P0->OUTCLR = (1 << 25);
    for (unsigned int i = 0; i < 100000; i++) {
      __NOP();
    }
  };
}

void lf_rtc_start(void) {

  /* Start LFCLK in LFRC low power mode */
  NRF_CLOCK->LFRCMODE |= 1;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;

  /* Use RTC to wait for 1s until everything has settled */
  NRF_RTC0->CC[1] = 32768 / 2;
  NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE1_Msk;
  NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE1_Msk;
  NRF_RTC0->TASKS_CLEAR = 1;

  /* Allow pending interrupts to wakeup CPU */
  SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
  NRF_RTC0->TASKS_START = 1;

  NRF_RTC0->EVENTS_COMPARE[1] = 0;
  while (NRF_RTC0->EVENTS_COMPARE[1] == 0) {
    __WFE();
    __SEV();
    __WFE();
  };
  NRF_RTC0->EVENTS_COMPARE[1] = 0;
  NRF_RTC0->EVTENCLR = RTC_EVTENCLR_COMPARE1_Msk;
  NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE1_Msk;
  NVIC_ClearPendingIRQ(RTC0_IRQn);

  /* Disable pending interrupts waking up CPU */
  SCB->SCR &= ~SCB_SCR_SEVONPEND_Msk;
}

void c_startup(void) {
  volatile unsigned long *src, *dst;

  /* Configure and set high pin P0_24 */
  NRF_P0->PIN_CNF[24] = 0x03;
  NRF_P0->OUTSET = (1 << 24);

  /* Start the RTC while waiting for cap voltage to recover from reset */
  lf_rtc_start();

  /* Poweroff unused RAM sections */
  for (unsigned int i = 1; i < 8; i++)
    NRF_POWER->RAM[i].POWERCLR = 0xFFFFFFFF;

  src = &_etext;
  dst = &_sdata;
  while (dst < &_edata)
    *(dst++) = *(src++);

  src = &_sbss;
  while (src < &_ebss)
    *(src++) = 0;

  /* Copy Vector Table from Flash to RAM */
  src = &_isr_vector_start;
  dst = ram_vector_table;
  while (src < &_isr_vector_end)
    *(dst++) = *(src++);

  /* Relocate Vector Table to RAM */
  SCB->VTOR = (uint32_t)ram_vector_table;
  __DSB();

  /* Clear pin P0_24 */
  NRF_P0->OUTCLR = (1 << 24);

  main();
}
