//******************************************************************************
// FXUTIL.H -- FlasherX utility functions
//******************************************************************************
#ifndef FXUTIL_H_
#define FXUTIL_H_

#include <Stream.h>
#include "CRCStream.h"

void read_ascii_line( Stream *serial, char *line, int maxbytes, bool* streamHasCRLF );
void update_firmware( CRCStream *in, Stream *out,
			uint32_t buffer_addr, uint32_t buffer_size );

#endif
