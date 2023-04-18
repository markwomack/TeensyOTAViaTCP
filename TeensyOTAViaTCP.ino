//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

//
// This is an example of updating the Teensy firmware using the FlasherX
// library after receiving the new firmware binary via TCP.
// This example will listen to a TCP port for the data. You can send
// the data using the following command in Linux (Ubuntu):
//   netcat -N [IP ADDRESS] [PORT] <data.txt
//  where data.txt is a file that contains the data.
//

// Arduino includes
#include <Arduino.h>

// My includes
#include <DebugMsgs.h>   // https://github.com/markwomack/ArduinoLogging
#include <TaskManager.h> // https://github.com/markwomack/TaskManager

// Third party includes
#include <WiFiServer.h>
#include <WiFiClient.h>

// Local includes
#include "pin_assignments.h"
#include "constants.h"
#include "MyNetworkHub.h"
#include "FXUtil.h"
extern "C" {
  #include "FlashTxx.h"    // TLC/T3x/T4x/TMM flash primitives
}

MyNetworkHub networkHub;

class CheckForOTATask : public Task {
  public:
    void setTCPServer(WiFiServer* tcpServer) {
      _tcpServer = tcpServer;
    }

    WiFiClient getTCPClient(void) {
      return _tcpClient;
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
};
CheckForOTATask checkForOTATask;

class CounterTask : public Task {
  public:
    void start(void) {
      _counter = 0;
    };

    void update(void) {
      DebugMsgs.debug().print("Counter: ").println(++_counter);
    };

  private:
    int _counter;
};
CounterTask counterTask;

void performUpdate(WiFiClient tcpClient) {
  uint32_t buffer_addr;
  uint32_t buffer_size;

  // create flash buffer to hold new firmware
  if (firmware_buffer_init( &buffer_addr, &buffer_size ) == 0) {
    Serial.printf( "unable to create buffer\n" );
    Serial.flush();
    return;
  }
  
  Serial.printf( "created buffer = %1luK %s (%08lX - %08lX)\n",
   buffer_size/1024, IN_FLASH(buffer_addr) ? "FLASH" : "RAM",
    buffer_addr, buffer_addr + buffer_size );

  update_firmware((Stream*)&tcpClient, (Stream*)&Serial, buffer_addr, buffer_size);
  
  // return from update_firmware() means error or user abort, so clean up and
  // reboot to ensure that static vars get boot-up initialized before retry
  Serial.printf( "erase FLASH buffer / free RAM buffer...\n" );
  firmware_buffer_free( buffer_addr, buffer_size );
  Serial.flush();
  REBOOT;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {;}
  
  DebugMsgs.enableLevel(DEBUG);

  pinMode(LED_STATUS_PIN, OUTPUT);

  // Connect to WiFi network
  if (networkHub.start() == 0) {
    WiFiServer* tcpServer = networkHub.getTCPServer(TCP_SERVER_PORT);
    checkForOTATask.setTCPServer(tcpServer);
  } else {
    DebugMsgs.debug().println("Unable to connect to WiFi!");
    while (true) {;}
  }

  //taskManager.addTask(&counterTask, 500);
  taskManager.addTask(&checkForOTATask, 1000);
  taskManager.addBlinkTask(LED_STATUS_PIN, 500);
  taskManager.start();
}

void loop() {
  // Normal processing
  taskManager.update();

  // If there is an OTA, stop everything and process
  if (checkForOTATask.otaIsAvailable()) {
    DebugMsgs.debug().println("OTA available, processing...");

    // stop normal processing
    taskManager.stop();

    // perform the OTA update
    performUpdate(checkForOTATask.getTCPClient());
  }
}
