//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//
 
#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <WiFiClient.h>

class OTAHandler {
  public:
    boolean readOTABinary(WiFiClient tcpCLient);
    void performUpdate(void);
};

#endif // OTA_HANDLER_H
