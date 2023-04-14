//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

#include "inttypes.h"

#include "utility.h"

#define RESTART_ADDR 0xE000ED0C
#define READ_RESTART() (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

void restartTeensy(void) {
  // Please see: https://forum.pjrc.com/threads/44857-How-to-Reset-Restart-Teensy-3-5-using-sotware
  WRITE_RESTART(0x5FA0004);
}
