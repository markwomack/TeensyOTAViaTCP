//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

//
// This is a verbose example of updating a Teensy's firmware via
// TCP using the FlasherX code from Joe Pasquariello (see github
// link below).
// This code will:
//   - Connect to a WiFi network.
//   - Open a server on a TCP port.
//   - Start normal operations which will periodically check the
//     TCP port for available data.
//   - When data is detected on the TCP port, normal operations
//     are suspended, and the data is read from the TCP port.
//   - The data is processed and stored in flash memory.
//   - If the data appears to be correct, then it is copied into
//     the Teensy firmware and the Teensy is restarted.
//   - After the Teensy restarts it should be running the new
//     program.
//
//  Please see the README for more details.

// Arduino includes
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

// My includes
#include <DebugMsgs.h>   // https://github.com/markwomack/ArduinoLogging
#include <TaskManager.h> // https://github.com/markwomack/TaskManager

// Local includes
#include "pin_assignments.h"
#include "constants.h"
#include "MyNetworkHub.h"
#include "FXUtil.h"     // https://github.com/joepasquariello/FlasherX
extern "C" {
  #include "FlashTxx.h"    // TLC/T3x/T4x/TMM flash primitives
}

void performUpdate(WiFiClient tcpClient);
CRCStream* getCRCStream(WiFiClient* tcpClient);

MyNetworkHub networkHub;

// This is a task that checks a TCP port for avialable
// data, and raises a flag to indicate that an update
// is available.
class CheckForOTATask : public Task {
  public:
    void setTCPServer(WiFiServer* tcpServer) {
      _tcpServer = tcpServer;
    }

    WiFiClient* getTCPClient(void) {
      return &_tcpClient;
    }
    
    void start(void) { 
      if (_tcpServer == 0) {
        DebugMsgs.debug().println("TCP Server not specified!");
      }
      
      _otaIsAvailable = false;
    }
    
    void update(void) {
      WiFiClient tcpClient = _tcpServer->available();

      // If a client was returned, store it for future use
      // and indicate an OTA is now available
      if (tcpClient) {
        _tcpClient = tcpClient;
        _otaIsAvailable = true;
      }
    }
    
    boolean otaIsAvailable(void) {
      return _otaIsAvailable;
    }

  private:
    WiFiServer* _tcpServer = 0;
    WiFiClient _tcpClient = 0;
    boolean _otaIsAvailable;
} checkForOTATask;

void setup() {
  Serial.begin(115200);
  while (!Serial) {;}
  
  DebugMsgs.enableLevel(DEBUG);

  pinMode(LED_STATUS_PIN, OUTPUT);

  // Connect to WiFi network, create a TCP port to monitor
  if (networkHub.start() == 0) {
    WiFiServer* tcpServer = networkHub.getTCPServer(TCP_SERVER_PORT);
    checkForOTATask.setTCPServer(tcpServer);
  } else {
    DebugMsgs.debug().println("Unable to connect to WiFi!");
    while (true) {;}
  }

  // This task will check the TCP port for an OTA update every second
  taskManager.addTask(&checkForOTATask, 1000);

  // This LED will blink (half second) during normal operations of the sketch
  taskManager.addBlinkTask(LED_STATUS_PIN, 500);

  // Start normal operations
  taskManager.start();
}

void loop() {
  // Normal operation
  taskManager.update();

  // If there is an OTA, stop everything and process
  if (checkForOTATask.otaIsAvailable()) {
    DebugMsgs.debug().println("OTA update available, processing...");

    // stop normal operation
    taskManager.stop();

    WiFiClient* tcpClient = checkForOTATask.getTCPClient();
    CRCStream* updateStream = getCRCStream(tcpClient);

    if (!updateStream) {
      tcpClient->stop();

      // Soft abort, can restart TaskManager
      taskManager.start();
      
      return;
    }
    
    // perform the OTA update
    performUpdate(updateStream);
  }
}

// Performs the actual firmware update using the FlasherX
// methods.
void performUpdate(CRCStream* updateStream) {
  uint32_t buffer_addr;
  uint32_t buffer_size;

  // create flash buffer to hold new firmware
  if (firmware_buffer_init( &buffer_addr, &buffer_size ) == 0) {
    DebugMsgs.debug().println("unable to create buffer");
    DebugMsgs.flush();
    return;
  }
  
  Serial.printf( "created buffer = %1luK %s (%08lX - %08lX)\n",
   buffer_size/1024, IN_FLASH(buffer_addr) ? "FLASH" : "RAM",
    buffer_addr, buffer_addr + buffer_size );

  update_firmware(updateStream, (Stream*)&Serial, buffer_addr, buffer_size);
  
  // return from update_firmware() means error, so clean up and
  // reboot to ensure that static vars get boot-up initialized before retry
  DebugMsgs.debug().println("erase FLASH buffer / free RAM buffer...restarting in 5 seconds");
  firmware_buffer_free( buffer_addr, buffer_size );
  DebugMsgs.flush();
  delay(5000);
  REBOOT;
}

CRCStream* getCRCStream(WiFiClient* tcpClient) {
  DebugMsgs.debug().println("Reading expected size and CRC");

  uint32_t expectedSize = 0;
  uint32_t expectedCRC = 0;
  
  uint8_t buffer[25];
  int count = 0;
  while (tcpClient->available()) {
    int data = tcpClient->read();
    if (data != -1) {
      if (data == '!') {
        buffer[count] = 0;
        expectedSize = strtoul((const char*)buffer, 0, 10);
        break;
      } else {
        buffer[count++] = (uint8_t)data;
      }
    }
  }

  if (expectedSize == 0) {
    DebugMsgs.debug().println("Error reading the expected size, aborted");
    return 0;
  }

  count = 0;
  while (tcpClient->available()) {
    count += tcpClient->readBytes(buffer + count, 9 - count);
    if (count == 9) {
      if (buffer[8] == '!') {
        buffer[8] = 0;
        expectedCRC = strtoul((const char*)buffer, 0, 16);
      }
      break;
    }
  }

  if (expectedCRC == 0) {
    DebugMsgs.error().println("Error reading the expected crc, aborted");
    return 0;
  }

  DebugMsgs.debug().printfln("Expected size: %d, expected CRC: %x", expectedSize, expectedCRC);
  
  return new CRCStream(tcpClient, expectedSize, expectedCRC);
}
