#ifndef ARDUINO_MSPM0_SPIDMA
#define ARDUINO_MSPM0_SPIDMA

#if defined(USE_ST7789)

#include "Arduino_DataBus.h"
#undef GFX_INLINE
#define GFX_INLINE

#include "SPI.h"

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

#ifndef MSPM0SPIDMA_MAX_PIXELS_AT_ONCE
#define MSPM0SPIDMA_MAX_PIXELS_AT_ONCE 1024
#endif

class Arduino_MSPM0SPIDMA : public Arduino_DataBus {
public:
  /**
   * @brief Constructor for MSPM0 SPI DMA Databus
   * @param spi_inst Pointer to the SPI register instance (e.g., SPI_0_INST)
   * @param dma_tx_ch DMA channel ID for SPI TX (e.g., DMA_CH0_CHAN_ID)
   * @param dcPort Data/Command GPIO Port (e.g., GPIOA)
   * @param dcPin Data/Command GPIO Pin Mask (e.g., DL_GPIO_PIN_15)
   * @param csPort Chip Select GPIO Port (e.g., GPIOB)
   * @param csPin Chip Select GPIO Pin Mask
   */
  Arduino_MSPM0SPIDMA(SPI_Regs *spi_inst, int8_t dma_tx_ch, GPIO_Regs *dcPort = nullptr, uint32_t dcPin = 0,
                      GPIO_Regs *csPort = nullptr, uint32_t csPin = 0);

  bool begin(int32_t speed = GFX_NOT_DEFINED, int8_t dataMode = SPI_MODE0) override;
  void beginWrite() override;
  void endWrite() override;
  void writeCommand(uint8_t) override;
  void writeCommand16(uint16_t) override;
  void writeCommandBytes(uint8_t *data, uint32_t len) override;
  void write(uint8_t) override;
  void write16(uint16_t) override;

  void writeC8D8(uint8_t c, uint8_t d) override;
  void writeC8D16(uint8_t c, uint16_t d) override;
  void writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2) override;

  void writeRepeat(uint16_t p, uint32_t len) override;
  void writePixels(uint16_t *data, uint32_t len) override;

  void writeBytes(uint8_t *data, uint32_t len) override;

  void writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len) override;
  void writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len) override;
  void writeYCbCrPixels(uint8_t *yData, uint8_t *cbData, uint8_t *crData, uint16_t w, uint16_t h) override;

  void transaction_complete();

protected:
  void flush_data_buf();
  GFX_INLINE void WRITE8BIT(uint8_t d);
  GFX_INLINE void WRITE9BIT(uint32_t d);
  GFX_INLINE void DC_HIGH(void);
  GFX_INLINE void DC_LOW(void);
  GFX_INLINE void CS_HIGH(void);
  GFX_INLINE void CS_LOW(void);
  GFX_INLINE void POLL_START();
  GFX_INLINE void POLL_END();

private:
  SPI_Regs *_spi_inst;
  int8_t _dma_tx_ch;

  GPIO_Regs *_dcPort;
  uint32_t _dcPin;
  GPIO_Regs *_csPort;
  uint32_t _csPin;

  // Variables mapping to standard ESP32 spi_tran struct behavior
  uint8_t *_spi_tran_tx_buffer;
  uint32_t _spi_tran_length; // in bits
  uint8_t _spi_tran_tx_data[4];
  bool _spi_tran_use_txdata;

  // Statically allocated 16-byte aligned buffers for DMA transfers
  alignas(16) union {
    uint8_t _buffer[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE * 2];
    uint16_t _buffer16[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE];
    uint32_t _buffer32[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE / 2];
  };

  alignas(16) union {
    uint8_t _2nd_buffer[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE * 2];
    uint16_t _2nd_buffer16[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE];
    uint32_t _2nd_buffer32[MSPM0SPIDMA_MAX_PIXELS_AT_ONCE / 2];
  };

  uint16_t _data_buf_bit_idx = 0;

  volatile bool _spi_ready;
};

#endif

#endif // ARDUINO_MSPM0_SPIDMA