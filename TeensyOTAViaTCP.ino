//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

//
// This is an example of updating the Teensy firmware using the FlasherX
// library after receiving the new firmware binary via TCP.
// This example will listen to a TCP port for the data. You can send
// the data using the following command in Linux (Ubuntu):
//   netcat -N 192.168.86.33 50005 <data.txt
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
#include "utility.h"
#include "MyNetworkHub.h"

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

  taskManager.addTask(&checkForOTATask, 1000);
  taskManager.addBlinkTask(LED_STATUS_PIN, 500);
  taskManager.start();
}

void loop() {
  taskManager.update();

  if (checkForOTATask.otaIsAvailable()) {
    // do the OTA update
    DebugMsgs.debug().println("OTA available, processing...");

    WiFiClient tcpClient = checkForOTATask.getTCPClient();

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
    delay(2000);

    networkHub.stop();
    
    DebugMsgs.debug().println("Restarting in 5 seconds");
    Serial.flush();
    delay(5000);

    // This isn't final, it just restarts the code by restarting the teensy for testing purposes
    restartTeensy();
  }
}
