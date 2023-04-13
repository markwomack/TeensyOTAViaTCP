//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
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

MyNetworkHub networkHub;

class CheckForOTATask : public Task {
  public:
    void setTCPServer(WiFiServer* tcpServer) {
      _tcpServer = tcpServer;
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
    WiFiClient _tcpClient;
    boolean _otaIsAvailable;
};
CheckForOTATask checkForOTATask;

void setup() {
  Serial.begin(9600);

  DebugMsgs.enableLevel(DEBUG);

  pinMode(LED_STATUS_PIN, OUTPUT);

  // Connect to WiFi network
  if (networkHub.start() == 0) {
    WiFiServer* tcpServer = networkHub.getTCPServer(TCP_SERVER_PORT);
    checkForOTATask.setTCPServer(tcpServer);
  } else {
    DebugMsgs.debug().println("Unable to connect to WiFi!");
    while (true) {;;}
  }

  taskManager.addBlinkTask(LED_STATUS_PIN, 500);
  taskManager.start();
}

void loop() {
  taskManager.update();

  if (checkForOTATask.otaIsAvailable()) {
    // do the OTA update
    DebugMsgs.debug().println("OTA available, processing...");
  }
}
