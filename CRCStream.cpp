//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

// C includes
#include <stdint.h>
#include <stdlib.h>

// Local includes
#include "CRCStream.h"

CRCStream::CRCStream(Stream* stream, uint32_t expectedSize, uint32_t expectedCRC) {
  _stream = stream;
  _expectedSize = expectedSize;
  _expectedCRC = expectedCRC;
  _currentSize = 0;
  _currentCRC = 0;
}

int CRCStream::available() {
  return _stream->available();
}

int CRCStream::read() {
  int data = _stream->read();
  
  if (data != -1) {
    crc32((uint8_t*)&data, 1, &_currentCRC);
    _currentSize++;
  }
  
  return data;
}
    
int CRCStream::peek() {
  return _stream->peek();
}

size_t CRCStream::write(uint8_t b) {
  return _stream->write(b);
}
    
bool CRCStream::sizeAndCRCMatch() {
  return (_expectedSize == _currentSize) && (_expectedCRC == _currentCRC);
}

uint32_t CRCStream::getExpectedSize() {
  return _expectedSize;
}

uint32_t CRCStream::getCurrentSize() {
  return _currentSize;
}

uint32_t CRCStream::getExpectedCRC() {
  return _expectedCRC;
}

uint32_t CRCStream::getCurrentCRC() {
  return _currentCRC;
}

// The following methods are originally from:
//     http://home.thep.lu.se/~bjorn/crc/
// All credit to the author.

uint32_t CRCStream::crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void CRCStream::crc32(const void *data, size_t n_bytes, uint32_t* crc) {
  static uint32_t table[0x100];
  if(!*table)
    for(size_t i = 0; i < 0x100; ++i)
      table[i] = crc32_for_byte(i);
  for(size_t i = 0; i < n_bytes; ++i)
    *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}
