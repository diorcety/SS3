#include "Arduino_MSPM0SPIDMA.h"

#if defined(USE_ST7789)

/**
 * @brief Arduino_MSPM0SPIDMA Constructor
 */
Arduino_MSPM0SPIDMA::Arduino_MSPM0SPIDMA(SPI_Regs *spi_inst, int8_t dma_tx_ch, GPIO_Regs *dcPort, uint32_t dcPin,
                                         GPIO_Regs *csPort, uint32_t csPin)
    : _spi_inst(spi_inst), _dma_tx_ch(dma_tx_ch), _dcPort(dcPort), _dcPin(dcPin), _csPort(csPort), _csPin(csPin) {
  (void)(_dma_tx_ch);
}

/**
 * @brief begin
 */
bool Arduino_MSPM0SPIDMA::begin(int32_t speed, int8_t dataMode) {
  (void)(speed);
  (void)(dataMode);

  DC_HIGH();
  CS_HIGH();

  return true;
}

void Arduino_MSPM0SPIDMA::beginWrite() {
  _data_buf_bit_idx = 0;
  _buffer[0] = 0;

  DC_HIGH();
  CS_LOW();
}

void Arduino_MSPM0SPIDMA::endWrite() {
  if (_data_buf_bit_idx > 0) {
    flush_data_buf();
  }
  CS_HIGH();
}

void Arduino_MSPM0SPIDMA::writeCommand(uint8_t c) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(c);
  } else {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    DC_LOW();

    _spi_tran_length = 8;
    _spi_tran_tx_data[0] = c;
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();

    DC_HIGH();
  }
}

void Arduino_MSPM0SPIDMA::writeCommand16(uint16_t c) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    _data16.value = c;
    WRITE9BIT(_data16.msb);
    WRITE9BIT(_data16.lsb);
  } else {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    DC_LOW();

    _spi_tran_length = 16;
    MSB_16_SET(_spi_tran_tx_data[0], c);
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();

    DC_HIGH();
  }
}

void Arduino_MSPM0SPIDMA::writeCommandBytes(uint8_t *data, uint32_t len) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    while (len--) {
      WRITE9BIT(*data++);
    }
  } else {
    DC_LOW();
    while (len--) {
      WRITE8BIT(*data++);
    }
    DC_HIGH();
  }
}

void Arduino_MSPM0SPIDMA::write(uint8_t d) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(0x100 | d);
  } else {
    WRITE8BIT(d);
  }
}

void Arduino_MSPM0SPIDMA::write16(uint16_t d) {
  _data16.value = d;
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(0x100 | _data16.msb);
    WRITE9BIT(0x100 | _data16.lsb);
  } else {
    WRITE8BIT(_data16.msb);
    WRITE8BIT(_data16.lsb);
  }
}

void Arduino_MSPM0SPIDMA::writeC8D8(uint8_t c, uint8_t d) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(c);
    WRITE9BIT(0x100 | d);
  } else {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    DC_LOW();

    _spi_tran_length = 8;
    _spi_tran_tx_data[0] = c;
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();

    DC_HIGH();

    _spi_tran_length = 8;
    _spi_tran_tx_data[0] = d;
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();
  }
}

void Arduino_MSPM0SPIDMA::writeC8D16(uint8_t c, uint16_t d) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(c);
    _data16.value = d;
    WRITE9BIT(0x100 | _data16.msb);
    WRITE9BIT(0x100 | _data16.lsb);
  } else {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    DC_LOW();

    _spi_tran_length = 8;
    _spi_tran_tx_data[0] = c;
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();

    DC_HIGH();

    _spi_tran_length = 16;
    _spi_tran_tx_data[0] = (d >> 8);
    _spi_tran_tx_data[1] = (d & 0xff);
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();
  }
}

void Arduino_MSPM0SPIDMA::writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    WRITE9BIT(c);
    _data16.value = d1;
    WRITE9BIT(0x100 | _data16.msb);
    WRITE9BIT(0x100 | _data16.lsb);
    _data16.value = d2;
    WRITE9BIT(0x100 | _data16.msb);
    WRITE9BIT(0x100 | _data16.lsb);
  } else {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    DC_LOW();

    _spi_tran_length = 8;
    _spi_tran_tx_data[0] = c;
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();

    DC_HIGH();

    _spi_tran_length = 32;
    _spi_tran_tx_data[0] = (d1 >> 8);
    _spi_tran_tx_data[1] = (d1 & 0xff);
    _spi_tran_tx_data[2] = (d2 >> 8);
    _spi_tran_tx_data[3] = (d2 & 0xff);
    _spi_tran_use_txdata = true;

    POLL_START();
    POLL_END();
  }
}

void Arduino_MSPM0SPIDMA::writeRepeat(uint16_t p, uint32_t len) {
  if (_data_buf_bit_idx > 0) {
    flush_data_buf();
  }

  if (_dcPort == nullptr) // 9-bit SPI
  {
    _data16.value = p;
    uint32_t hi = 0x100 | _data16.msb;
    uint32_t lo = 0x100 | _data16.lsb;
    uint16_t idx;
    uint8_t shift;
    uint16_t bufLen = (len <= 28) ? len : 28;
    int16_t xferLen;
    for (uint32_t t = 0; t < bufLen; t++) {
      idx = _data_buf_bit_idx >> 3;
      shift = (_data_buf_bit_idx % 8);
      if (shift) {
        _buffer[idx++] |= hi >> (shift + 1);
        _buffer[idx] = hi << (7 - shift);
      } else {
        _buffer[idx++] = hi >> 1;
        _buffer[idx] = hi << 7;
      }
      _data_buf_bit_idx += 9;

      idx = _data_buf_bit_idx >> 3;
      shift = (_data_buf_bit_idx % 8);
      if (shift) {
        _buffer[idx++] |= lo >> (shift + 1);
        _buffer[idx] = lo << (7 - shift);
      } else {
        _buffer[idx++] = lo >> 1;
        _buffer[idx] = lo << 7;
      }
      _data_buf_bit_idx += 9;
    }

    while (len) {
      xferLen = (bufLen < len) ? bufLen : len;
      _data_buf_bit_idx = xferLen * 18;

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = _data_buf_bit_idx;
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= xferLen;
    }
  } else // 8-bit SPI
  {
    uint16_t bufLen = (len >= MSPM0SPIDMA_MAX_PIXELS_AT_ONCE) ? MSPM0SPIDMA_MAX_PIXELS_AT_ONCE : len;
    int16_t xferLen, l;
    uint32_t c32;
    MSB_32_16_16_SET(c32, p, p);

    l = (bufLen + 1) / 2;
    for (int16_t i = 0; i < l; i++) {
      _buffer32[i] = c32;
    }

    while (len) {
      xferLen = (bufLen <= len) ? bufLen : len;

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = xferLen << 4; // length in bits (xferLen * 16)
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= xferLen;
    }
  }

  _data_buf_bit_idx = 0;
}

void Arduino_MSPM0SPIDMA::writePixels(uint16_t *data, uint32_t len) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    while (len--) {
      write16(*data++);
    }
  } else // 8-bit SPI
  {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    uint32_t l, l2;
    uint16_t p1, p2;
    while (len) {
      l = (len > MSPM0SPIDMA_MAX_PIXELS_AT_ONCE) ? MSPM0SPIDMA_MAX_PIXELS_AT_ONCE : len;
      l2 = (l + 1) >> 1;
      for (uint32_t i = 0; i < l2; ++i) {
        p1 = *data++;
        p2 = *data++;
        MSB_32_16_16_SET(_buffer32[i], p1, p2);
      }
      if (l & 1) {
        p1 = *data++;
        MSB_16_SET(_buffer16[l - 1], p1);
      }

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = l << 4; // length in bits
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= l;
    }
  }
}

void Arduino_MSPM0SPIDMA::writeBytes(uint8_t *data, uint32_t len) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    while (len--) {
      write(*data++);
    }
  } else // 8-bit SPI
  {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    uint32_t l, l4;
    uint32_t *p;
    while (len) {
      l = (len > (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE << 1)) ? (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE << 1) : len;
      l4 = (l + 3) >> 2;
      p = (uint32_t *)data;
      for (uint32_t i = 0; i < l4; ++i) {
        _buffer32[i] = *p++;
      }

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = l << 3; // length in bits
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= l;
      data += l;
    }
  }
}

#if !defined(LITTLE_FOOT_PRINT)

void Arduino_MSPM0SPIDMA::writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    while (len--) {
      write16(idx[*data++]);
    }
  } else // 8-bit SPI
  {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    uint32_t l, l2;
    uint16_t p1, p2;
    while (len) {
      l = (len > MSPM0SPIDMA_MAX_PIXELS_AT_ONCE) ? MSPM0SPIDMA_MAX_PIXELS_AT_ONCE : len;
      l2 = l >> 1;
      for (uint32_t i = 0; i < l2; ++i) {
        p1 = idx[*data++];
        p2 = idx[*data++];
        MSB_32_16_16_SET(_buffer32[i], p1, p2);
      }
      if (l & 1) {
        p1 = idx[*data++];
        MSB_16_SET(_buffer16[l - 1], p1);
      }

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = l << 4;
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= l;
    }
  }
}

void Arduino_MSPM0SPIDMA::writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len) {
  if (_dcPort == nullptr) // 9-bit SPI
  {
    uint16_t hi, lo;
    while (len--) {
      _data16.value = idx[*data++];
      hi = 0x100 | _data16.msb;
      lo = 0x100 | _data16.lsb;
      WRITE9BIT(hi);
      WRITE9BIT(lo);
      WRITE9BIT(hi);
      WRITE9BIT(lo);
    }
  } else // 8-bit SPI
  {
    if (_data_buf_bit_idx > 0) {
      flush_data_buf();
    }

    uint32_t l;
    uint16_t p;
    while (len) {
      l = (len > (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE >> 1)) ? (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE >> 1) : len;
      for (uint32_t i = 0; i < l; ++i) {
        p = idx[*data++];
        MSB_32_16_16_SET(_buffer32[i], p, p);
      }

      _spi_tran_tx_buffer = (uint8_t *)_buffer32;
      _spi_tran_length = l << 5;
      _spi_tran_use_txdata = false;

      POLL_START();
      POLL_END();

      len -= l;
    }
  }
}

void Arduino_MSPM0SPIDMA::writeYCbCrPixels(uint8_t *yData, uint8_t *cbData, uint8_t *crData, uint16_t w, uint16_t h) {
  if (w > (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE / 2)) {
    Arduino_DataBus::writeYCbCrPixels(yData, cbData, crData, w, h);
  } else {
    int cols = w >> 1;
    int rows = h >> 1;
    uint8_t *yData2 = yData + w;
    uint16_t *dest = _buffer16;
    uint16_t *dest2 = dest + w;

    uint8_t pxCb, pxCr;
    int16_t pxR, pxG, pxB, pxY;

    uint16_t out_bits = w << 5;
    bool poll_started = false;
    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        pxCb = *cbData++;
        pxCr = *crData++;
        pxR = CR2R16[pxCr];
        pxG = -CB2G16[pxCb] - CR2G16[pxCr];
        pxB = CB2B16[pxCb];

        pxY = Y2I16[*yData++];
        *dest++ = CLIPRBE[pxY + pxR] | CLIPGBE[pxY + pxG] | CLIPBBE[pxY + pxB];
        pxY = Y2I16[*yData++];
        *dest++ = CLIPRBE[pxY + pxR] | CLIPGBE[pxY + pxG] | CLIPBBE[pxY + pxB];
        pxY = Y2I16[*yData2++];
        *dest2++ = CLIPRBE[pxY + pxR] | CLIPGBE[pxY + pxG] | CLIPBBE[pxY + pxB];
        pxY = Y2I16[*yData2++];
        *dest2++ = CLIPRBE[pxY + pxR] | CLIPGBE[pxY + pxG] | CLIPBBE[pxY + pxB];
      }
      yData += w;
      yData2 += w;

      if (poll_started) {
        POLL_END();
      } else {
        poll_started = true;
      }
      if (row & 1) {
        _spi_tran_tx_buffer = (uint8_t *)_2nd_buffer32;
        dest = _buffer16;
      } else {
        _spi_tran_tx_buffer = (uint8_t *)_buffer32;
        dest = _2nd_buffer16;
      }
      _spi_tran_length = out_bits;
      _spi_tran_use_txdata = false;

      POLL_START();
      dest2 = dest + w;
    }

    POLL_END();
  }
}

#endif

void Arduino_MSPM0SPIDMA::flush_data_buf() {
  _spi_tran_tx_buffer = (uint8_t *)_buffer32;
  _spi_tran_length = _data_buf_bit_idx;
  _spi_tran_use_txdata = false;

  POLL_START();
  POLL_END();

  _data_buf_bit_idx = 0;
}

GFX_INLINE void Arduino_MSPM0SPIDMA::WRITE8BIT(uint8_t d) {
  uint16_t idx = _data_buf_bit_idx >> 3;
  _buffer[idx] = d;
  _data_buf_bit_idx += 8;
  if (_data_buf_bit_idx >= (MSPM0SPIDMA_MAX_PIXELS_AT_ONCE << 4)) {
    flush_data_buf();
  }
}

GFX_INLINE void Arduino_MSPM0SPIDMA::WRITE9BIT(uint32_t d) {
  uint16_t idx = _data_buf_bit_idx >> 3;
  uint8_t shift = (_data_buf_bit_idx % 8);
  if (shift) {
    _buffer[idx++] |= d >> (shift + 1);
    _buffer[idx] = d << (7 - shift);
  } else {
    _buffer[idx++] = d >> 1;
    _buffer[idx] = d << 7;
  }
  _data_buf_bit_idx += 9;
  if (_data_buf_bit_idx >= 504) // 56 bytes * 9 bits
  {
    flush_data_buf();
  }
}

/******** low level bit twiddling **********/

GFX_INLINE void Arduino_MSPM0SPIDMA::DC_HIGH(void) {
  if (_dcPort != nullptr)
    DL_GPIO_setPins(_dcPort, _dcPin);
}

GFX_INLINE void Arduino_MSPM0SPIDMA::DC_LOW(void) {
  if (_dcPort != nullptr)
    DL_GPIO_clearPins(_dcPort, _dcPin);
}

GFX_INLINE void Arduino_MSPM0SPIDMA::CS_HIGH(void) {
  if (_csPort != nullptr)
    DL_GPIO_setPins(_csPort, _csPin);
}

GFX_INLINE void Arduino_MSPM0SPIDMA::CS_LOW(void) {
  if (_csPort != nullptr)
    DL_GPIO_clearPins(_csPort, _csPin);
}

//#define DISABLE_DMA

GFX_INLINE void Arduino_MSPM0SPIDMA::POLL_START() {
  uint32_t bytes = (_spi_tran_length + 7) / 8; // Convert bits to bytes

  if (_spi_tran_use_txdata) {
    // Fast Polled I/O for 1-4 bytes (e.g. single Commands/Data)
    for (uint32_t i = 0; i < bytes; i++) {
      while (DL_SPI_isTXFIFOFull(_spi_inst)) {
        yield();
      }
      DL_SPI_transmitData8(_spi_inst, _spi_tran_tx_data[i]);
    }
  } else {
#ifndef DISABLE_DMA
    // 1. ALWAYS disable the channel before changing its configuration
    DL_DMA_disableChannel(DMA, _dma_tx_ch);

    // Hardware DMA Trigger for Buffers
    DL_DMA_setSrcAddr(DMA, _dma_tx_ch, (uint32_t)_spi_tran_tx_buffer);
    DL_DMA_setDestAddr(DMA, _dma_tx_ch, (uint32_t)(&_spi_inst->TXDATA));
    DL_DMA_setTransferSize(DMA, _dma_tx_ch, bytes);

    // Enabling channel will start transfer assuming SPI TX DMA request is active
    __DSB();
    DL_DMA_enableChannel(DMA, _dma_tx_ch);
#else
    for (uint32_t i = 0; i < bytes; i++) {
      while (DL_SPI_isTXFIFOFull(_spi_inst)) {
        yield();
      }
      DL_SPI_transmitData8(_spi_inst, _spi_tran_tx_buffer[i]);
    }
#endif
  }
}

GFX_INLINE void Arduino_MSPM0SPIDMA::POLL_END() {
#ifndef DISABLE_DMA
  if (!_spi_tran_use_txdata) {
    // Wait for the SPI peripheral to finish shifting all data out of its FIFO
    while (DL_DMA_isChannelEnabled(DMA, _dma_tx_ch)) {
      yield();
    }
  }
#endif

  while (DL_SPI_isBusy(_spi_inst)) {
    yield();
  }
}

#endif
