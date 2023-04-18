//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

//
// This is an implementation of a simple network hub based on
// a WiFi connection. This version requires the Adafruit version
// of the Arduino WiFiNINA library since the SPI pins are specified.
//

// My includes
#include <DebugMsgs.h>

// Third-party includes
#include <SPI.h>
#include <WiFiNINA.h> // https://github.com/arduino-libraries/WiFiNINA
#include <WiFiUdp.h>
#include <WiFiServer.h>

// Local includes
#include "MyNetworkHub.h"
#include "pin_assignments.h"
#include "secrets.h"

char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

void printWifiStatus(void);

int MyNetworkHub::start(void) {
  // Make sure the right pins are set for SPI
  SPI.setMOSI(WIFI_SPI_MOSI0_PIN);
  SPI.setMISO(WIFI_SPI_MISO0_PIN);
  SPI.setSCK(WIFI_SPI_SCK0_PIN);
  SPI.begin();

  pinMode(WIFI_BUSY_PIN, INPUT);
  pinMode(WIFI_RESET_PIN, OUTPUT);

  // Make sure right pins are set for Wifi
  WiFi.setPins(WIFI_SPI_CS0_PIN, WIFI_BUSY_PIN, WIFI_RESET_PIN, WIFI_GPIO_PIN);

  DebugMsgs.debug().print("Found firmware ").println(WiFi.firmwareVersion());
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    DebugMsgs.debug().println("Communication with WiFi module failed!");
    return 1;
  }

  int status = WL_IDLE_STATUS;
  int attemptsLeft = 3;
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    if (attemptsLeft == 0) {
      DebugMsgs.debug().println("All conection attempts exhausted, failed to connected to wifi");
      return 1;
    }
    
    DebugMsgs.debug().print("Attempting to connect to SSID: ").println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    attemptsLeft--;
    
    // wait 10 seconds for connection:
    delay(10000);
  }

  DebugMsgs.debug().println("Connected to wifi");
  printWifiStatus();

  return 0;
}

void MyNetworkHub::stop(void) {
  WiFi.end();
}

UDP* MyNetworkHub::getUdpPort(uint32_t portNum) {
  WiFiUDP* udp = new WiFiUDP();
  udp->begin(portNum);
  DebugMsgs.debug().print("Opened UDP Port: ").println(portNum);
  return udp;
}

WiFiServer* MyNetworkHub::getTCPServer(uint32_t portNum) {
  WiFiServer* tcpServer = new WiFiServer(portNum);
  tcpServer->begin();
  DebugMsgs.debug().print("Opened TCP Port: ").println(portNum);
  return tcpServer;
}

void printWifiStatus(void) {
  // print the SSID of the network you're attached to:
  DebugMsgs.debug().print("SSID: ").println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  DebugMsgs.debug().print("IP Address: ").println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DebugMsgs.debug().print("signal strength (RSSI):").print(rssi).println(" dBm");
}
