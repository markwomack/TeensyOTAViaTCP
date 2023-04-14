//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

// My includes
#include <DebugMsgs.h>

// Local includes
#include "utility.h"
#include "OTAHandler.h"
#include "MyNetworkHub.h"

boolean OTAHandler::readOTABinary(WiFiClient tcpClient) {
  DebugMsgs.debug().println("Reading OTA Binary");
  DebugMsgs.debug().println("Message:");
  
  uint32_t lastReadMillis = millis();
  boolean tcpConnectionIdle = false;
  while (!tcpConnectionIdle) {
    if (tcpClient.available() > 0) {
      DebugMsgs.print((char)tcpClient.read());
      lastReadMillis = millis();
    } else {
      if (millis() - lastReadMillis > 500) {
        tcpConnectionIdle = true;
      }
    }
  }
  DebugMsgs.debug().println("TCP message completed after timeout");
  Serial.flush();
  tcpClient.stop();
  return true;
}

void OTAHandler::performUpdate(void) {
  DebugMsgs.debug().println("Restarting in 5 seconds");
  Serial.flush();
  delay(5000);

  // This isn't final, it just restarts the code by restarting the teensy for testing purposes
  restartTeensy();
}
