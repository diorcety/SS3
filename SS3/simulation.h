#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* -----------------------------------------------------------------------
 * Stub logger
 * Compile with -DSIM_LOG_DISABLE to silence all stub traces.
 * SysTick_Handler is intentionally excluded (fires every 1 ms).
 * --------------------------------------------------------------------- */
#ifndef SIM_LOG_DISABLE
#define SIM_LOG(fmt, ...) fprintf(stderr, "[SIM] " fmt "\n", ##__VA_ARGS__)
#else
#define SIM_LOG(fmt, ...) ((void)0)
#endif

/* -----------------------------------------------------------------------
 * Generic opaque peripheral register types
 * --------------------------------------------------------------------- */
typedef struct {
  uint32_t reserved[64];
} ADC12_Regs;
typedef struct {
  uint32_t reserved[16];
  uint32_t CLKDIV;
  uint32_t TXDATA;
  uint32_t reserved2[47];
} SPI_Regs;
typedef struct {
  uint32_t reserved[64];
} OA_Regs;
typedef struct {
  uint32_t reserved[64];
} COMP_Regs;
typedef struct {
  uint32_t reserved[64];
} WWDT_Regs;
typedef struct {
  uint32_t reserved[64];
} GPIO_Regs;
typedef struct {
  uint32_t reserved[64];
} TIMERA_Regs;
typedef struct {
  uint32_t reserved[64];
} DMA_Regs;

/* -----------------------------------------------------------------------
 * System / NVIC stubs
 * --------------------------------------------------------------------- */
typedef int IRQn_Type;

void SYSCFG_DL_init(void); /* defined in sim_systick.c */

static inline void NVIC_EnableIRQ(IRQn_Type irq) { SIM_LOG("NVIC_EnableIRQ(irq=%d)", irq); }
static inline void NVIC_DisableIRQ(IRQn_Type irq) { SIM_LOG("NVIC_DisableIRQ(irq=%d)", irq); }
static inline void NVIC_ClearPendingIRQ(IRQn_Type irq) { SIM_LOG("NVIC_ClearPendingIRQ(irq=%d)", irq); }
static inline void __disable_irq(void) { SIM_LOG("__disable_irq()"); }
static inline void __enable_irq(void) { SIM_LOG("__enable_irq()"); }

/* -----------------------------------------------------------------------
 * Board-level instance / pin constants  (from SysConfig / ti_msp_dl_config)
 * --------------------------------------------------------------------- */

/* ADC instances */
static ADC12_Regs _ADCRight_inst;
static ADC12_Regs _ADCLeft_inst;
#define ADCRight_INST (&_ADCRight_inst)
#define ADCLeft_INST (&_ADCLeft_inst)

/* ADC memory channel indices */
#define ADCRight_ADCMEM_TC1A 0
#define ADCRight_ADCMEM_TC1V 1
#define ADCRight_ADCMEM_TC1 2
#define ADCLeft_ADCMEM_TC2A 0
#define ADCLeft_ADCMEM_TC2V 1
#define ADCLeft_ADCMEM_TC2 2

/* ADC reference voltages (volts, adjust to match your hardware) */
#define ADCRight_ADCMEM_TC1A_REF_VOLTAGE_V 3.3f
#define ADCRight_ADCMEM_TC1V_REF_VOLTAGE_V 3.3f
#define ADCRight_ADCMEM_TC1_REF_VOLTAGE_V 3.3f
#define ADCLeft_ADCMEM_TC2A_REF_VOLTAGE_V 3.3f
#define ADCLeft_ADCMEM_TC2V_REF_VOLTAGE_V 3.3f
#define ADCLeft_ADCMEM_TC2_REF_VOLTAGE_V 3.3f

/* ADC interrupt mask bit */
#define ADC12_CPU_INT_IMASK_MEMRESIFG0_SET (1u)
#define ADC12_CTL2_STARTADD_OFS 4u

/* GPIO ports */
static GPIO_Regs _Buttons_PORT_inst;
static GPIO_Regs _Switches_PORT_inst;
static GPIO_Regs _Screen_PORT_inst;
static GPIO_Regs _Other_PORT_inst;
#define Buttons_PORT (&_Buttons_PORT_inst)
#define Switches_PORT (&_Switches_PORT_inst)
#define Screen_PORT (&_Screen_PORT_inst)
#define Other_PORT (&_Other_PORT_inst)

/* GPIO pin masks */
#define Buttons_A_PIN (1u << 0)
#define Buttons_B_PIN (1u << 1)
#define Buttons_P_PIN (1u << 2)
#define Buttons_INT_IRQN ((IRQn_Type)0)

#define Switches_R24_PIN (1u << 0)
#define Switches_R12_PIN (1u << 1)
#define Switches_L24_PIN (1u << 2)
#define Switches_L12_PIN (1u << 3)

#define Screen_Reset_PIN (1u << 0)
#define Screen_BL_PIN (1u << 1)
#define Screen_CMD_PIN (1u << 2)

#define Other_REED_PULLUP_PIN (1u << 0)

/* OPA instance */
static OA_Regs _SecondVRef_inst;
#define SecondVRef_INST (&_SecondVRef_inst)
#define OA_CFG_OUTPIN_DISABLED 0u
#define OA_CFG_OUTPIN_ENABLED 1u

/* COMP / ZCD instance */
static COMP_Regs _ZCD_inst;
#define ZCD_INST (&_ZCD_inst)
#define ZCD_INST_INT_IRQN ((IRQn_Type)1)
#define DL_COMP_INTERRUPT_OUTPUT_EDGE (1u << 0)
#define DL_COMP_INTERRUPT_OUTPUT_EDGE_INV (1u << 1)

/* WWDT / power-protection instance */
static WWDT_Regs _PowerProtection_inst;
#define PowerProtection_INST (&_PowerProtection_inst)
#define PowerProtection_INT_IRQN ((IRQn_Type)2)

/* Timer instance */
static TIMERA_Regs _MeasurementTimer_inst;
#define MeasurementTimer_INST (&_MeasurementTimer_inst)
#define MeasurementTimer_INST_INT_IRQN ((IRQn_Type)3)
#define DL_TIMERA_INTERRUPT_ZERO_EVENT 1u

/* SPI instance + constants */
static SPI_Regs _SPI_0_inst;
static GPIO_Regs _SPI_0_PORT_inst;
#define SPI_0_INST (&_SPI_0_inst)
#define DL_SPI_CLOCK_DIVIDE_RATIO_6 5
#define DL_SPI_DATA_SIZE_16 16u
#define SPI_CLKCTL_SCR_MASK 0x000003FFU
#define SPI_0_DMA_TX_CHAN_ID 0
#define GPIO_SPI_0_IOMUX_CS1 0
#define IOMUX_PINCM8_PF_GPIOA_DIO03 3
#define GPIO_SPI_0_CS1_PORT (&_SPI_0_PORT_inst)
#define GPIO_SPI_0_CS1_PIN 3

/* CPU clock frequency */
#define CPUCLK_FREQ 32000000u

/* -----------------------------------------------------------------------
 * DL_GPIO stubs
 * --------------------------------------------------------------------- */
static inline uint32_t DL_GPIO_readPins(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_readPins(port=%p, pins=0x%08X) -> 0", (void *)port, pins);
  return 0u;
}
static inline void DL_GPIO_setPins(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_setPins(port=%p, pins=0x%08X)", (void *)port, pins);
}
static inline void DL_GPIO_clearPins(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_clearPins(port=%p, pins=0x%08X)", (void *)port, pins);
}
static inline void DL_GPIO_writePinsVal(GPIO_Regs *port, uint32_t pins, uint32_t val) {
  SIM_LOG("DL_GPIO_writePinsVal(port=%p, pins=0x%08X, val=0x%08X)", (void *)port, pins, val);
}
static inline void DL_GPIO_enableOutput(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_enableOutput(port=%p, pins=0x%08X)", (void *)port, pins);
}
static inline void DL_GPIO_disableOutput(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_disableOutput(port=%p, pins=0x%08X)", (void *)port, pins);
}
static inline void DL_GPIO_clearInterruptStatus(GPIO_Regs *port, uint32_t pins) {
  SIM_LOG("DL_GPIO_clearInterruptStatus(port=%p, pins=0x%08X)", (void *)port, pins);
}
static inline void DL_GPIO_initPeripheralOutputFunction(uint32_t pincmIndex, uint32_t function) {
  SIM_LOG("DL_GPIO_initPeripheralOutputFunction(pincmIndex=0x%08X, function=0x%08X)", pincmIndex, function);
}

/* -----------------------------------------------------------------------
 * DL_ADC12 stubs
 * --------------------------------------------------------------------- */
static inline void DL_ADC12_disableConversions(ADC12_Regs *adc) {
  SIM_LOG("DL_ADC12_disableConversions(adc=%p)", (void *)adc);
}
static inline void DL_ADC12_enableConversions(ADC12_Regs *adc) {
  SIM_LOG("DL_ADC12_enableConversions(adc=%p)", (void *)adc);
}
static inline void DL_ADC12_startConversion(ADC12_Regs *adc) {
  SIM_LOG("DL_ADC12_startConversion(adc=%p)", (void *)adc);
}
static inline void DL_ADC12_setStartAddress(ADC12_Regs *adc, uint32_t addr) {
  SIM_LOG("DL_ADC12_setStartAddress(adc=%p, addr=%u)", (void *)adc, addr);
}
static inline void DL_ADC12_clearInterruptStatus(ADC12_Regs *adc, uint32_t mask) {
  SIM_LOG("DL_ADC12_clearInterruptStatus(adc=%p, mask=0x%08X)", (void *)adc, mask);
}
static inline uint32_t DL_ADC12_getPendingInterrupt(ADC12_Regs *adc) {
  SIM_LOG("DL_ADC12_getPendingInterrupt(adc=%p) -> 0", (void *)adc);
  return 0u;
}
static inline uint32_t DL_ADC12_getEnabledInterruptStatus(ADC12_Regs *adc, uint32_t mask) {
  SIM_LOG("DL_ADC12_getEnabledInterruptStatus(adc=%p, mask=0x%08X) -> 0", (void *)adc, mask);
  return 0u;
}
static inline uint16_t DL_ADC12_getMemResult(ADC12_Regs *adc, uint32_t mem) {
  SIM_LOG("DL_ADC12_getMemResult(adc=%p, mem=%u) -> 0", (void *)adc, mem);
  return 0u;
}

/* -----------------------------------------------------------------------
 * DL_OPA stubs
 * --------------------------------------------------------------------- */
static inline void DL_OPA_setOutputPinState(OA_Regs *opa, uint32_t state) {
  SIM_LOG("DL_OPA_setOutputPinState(opa=%p, state=%u)", (void *)opa, state);
}

/* -----------------------------------------------------------------------
 * DL_COMP stubs
 * --------------------------------------------------------------------- */
static inline void DL_COMP_clearInterruptStatus(COMP_Regs *comp, uint32_t mask) {
  SIM_LOG("DL_COMP_clearInterruptStatus(comp=%p, mask=0x%08X)", (void *)comp, mask);
}
static inline uint32_t DL_COMP_getPendingInterrupt(COMP_Regs *comp) {
  SIM_LOG("DL_COMP_getPendingInterrupt(comp=%p) -> 0", (void *)comp);
  return 0u;
}

/* -----------------------------------------------------------------------
 * DL_WWDT stubs
 * --------------------------------------------------------------------- */
static inline bool DL_WWDT_getPendingInterrupt(WWDT_Regs *wwdt) {
  SIM_LOG("DL_WWDT_getPendingInterrupt(wwdt=%p) -> false", (void *)wwdt);
  return false;
}
static inline void DL_WWDT_clearInterruptStatus(WWDT_Regs *wwdt) {
  SIM_LOG("DL_WWDT_clearInterruptStatus(wwdt=%p)", (void *)wwdt);
}

/* -----------------------------------------------------------------------
 * DL_TimerA stubs
 * --------------------------------------------------------------------- */
static inline void DL_TimerA_startCounter(TIMERA_Regs *timer) {
  SIM_LOG("DL_TimerA_startCounter(timer=%p)", (void *)timer);
}
static inline void DL_TimerA_stopCounter(TIMERA_Regs *timer) {
  SIM_LOG("DL_TimerA_stopCounter(timer=%p)", (void *)timer);
}
static inline uint32_t DL_TimerA_getTimerCount(TIMERA_Regs *timer) {
  SIM_LOG("DL_TimerA_getTimerCount(timer=%p) -> 0", (void *)timer);
  return 0u;
}
static inline uint32_t DL_TimerA_getPendingInterrupt(TIMERA_Regs *timer) {
  SIM_LOG("DL_TimerA_getPendingInterrupt(timer=%p) -> 0", (void *)timer);
  return 0u;
}

/* -----------------------------------------------------------------------
 * DL_SPI stubs
 * --------------------------------------------------------------------- */
static inline void DL_SPI_setBitRateSerialClockDivider(SPI_Regs *spi, uint32_t div) {
  SIM_LOG("DL_SPI_setBitRateSerialClockDivider(spi=%p, div=%u)", (void *)spi, div);
}
static inline void DL_SPI_setDataSize(SPI_Regs *spi, uint32_t size) {
  SIM_LOG("DL_SPI_setDataSize(spi=%p, size=%u)", (void *)spi, size);
}
static inline void DL_SPI_transmitData16(SPI_Regs *spi, uint16_t data) {
  SIM_LOG("DL_SPI_transmitData16(spi=%p, data=0x%04X)", (void *)spi, data);
}
static inline void DL_SPI_transmitData8(SPI_Regs *spi, uint8_t data) {
  SIM_LOG("DL_SPI_transmitData16(spi=%p, data=0x%02X)", (void *)spi, data);
}
static inline bool DL_SPI_isBusy(SPI_Regs *spi) {
  SIM_LOG("DL_SPI_isBusy(spi=%p) -> false", (void *)spi);
  return false;
}
static inline bool DL_SPI_isTXFIFOEmpty(SPI_Regs *spi) {
  SIM_LOG("DL_SPI_isTXFIFOEmpty(spi=%p) -> true", (void *)spi);
  return true;
}
static inline bool DL_SPI_isTXFIFOFull(SPI_Regs *spi) {
  SIM_LOG("DL_SPI_isTXFIFOFull(spi=%p) -> true", (void *)spi);
  return false;
}

static DMA_Regs _DMA_Regs;
#define DMA (&_DMA_Regs)

static inline void DL_DMA_setSrcAddr(DMA_Regs *dma, uint8_t channelNum, uint32_t srcAddr) {
  SIM_LOG("DL_DMA_setSrcAddr(dma=%p, channelNum=0x%02X, srcAddr=0x%08X)", (void *)dma, channelNum, srcAddr);
}
static inline void DL_DMA_setDestAddr(DMA_Regs *dma, uint8_t channelNum, uint32_t destAddr) {
  SIM_LOG("DL_DMA_setDestAddr(dma=%p, channelNum=0x%02X, destAddr=0x%08X)", (void *)dma, channelNum, destAddr);
}
static inline bool DL_DMA_isChannelEnabled(DMA_Regs *dma, uint8_t channelNum) {
  SIM_LOG("DL_DMA_isChannelEnabled(dma=%p, channelNum=0x%02X) -> false", (void *)dma, channelNum);
  return false;
}
static inline void DL_DMA_enableChannel(DMA_Regs *dma, uint8_t channelNum) {
  SIM_LOG("DL_DMA_enableChannel(dma=%p, channelNum=0x%02X)", (void *)dma, channelNum);
}
static inline void DL_DMA_disableChannel(DMA_Regs *dma, uint8_t channelNum) {
  SIM_LOG("DL_DMA_disableChannel(dma=%p, channelNum=0x%02X)", (void *)dma, channelNum);
}
static inline void DL_DMA_setTransferSize(DMA_Regs *dma, uint8_t channelNum, uint16_t size) {
  SIM_LOG("DL_DMA_isChannelEnabled(dma=%p, channelNum=0x%02X, size=0x%04X)", (void *)dma, channelNum, size);
}

#define __DSB()

#endif /* SIMULATION_H_ */
