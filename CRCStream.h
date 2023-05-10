//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

#ifndef CRCSTREAM_H
#define CRCSTREAM_H

// Arduino includes
#include <Stream.h>

class CRCStream : public Stream {
  public:
    CRCStream(Stream* stream, uint32_t expectedSize, uint32_t expectedCRC);

    // Need to be implemented for Stream interface
    int available();
    int read();
    int peek();
    size_t write(uint8_t b);
    
    bool sizeAndCRCMatch();
    uint32_t getExpectedSize();
    uint32_t getCurrentSize();
    uint32_t getExpectedCRC();
    uint32_t getCurrentCRC();
    
  private:
    Stream* _stream;
    uint32_t _expectedSize;
    uint32_t _currentSize;
    uint32_t _expectedCRC;
    uint32_t _currentCRC;

    uint32_t crc32_for_byte(uint32_t r);
    void crc32(const void *data, size_t n_bytes, uint32_t* crc);
};
#endif // CRCSTREAM_H
