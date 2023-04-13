//
// Licensed under the MIT license.
// See accompanying LICENSE file for details.
//

// My includes
#include <DebugMsgs.h>

// Third-party includes
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

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

  // Make sure right pins are set for Wifi
  WiFi.setPins(WIFI_SPI_CS0_PIN, WIFI_BUSY_PIN, WIFI_RESET_PIN, -1, &SPI);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    DebugMsgs.println("Communication with WiFi module failed!");
    return 1;
  }

  int status = WL_IDLE_STATUS;
  int attemptsLeft = 3;
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    if (attemptsLeft == 0) {
      DebugMsgs.println("All conection attempts exhausted, failed to connected to wifi");
      return 1;
    }
    
    DebugMsgs.print("Attempting to connect to SSID: ").println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    attemptsLeft--;
    
    // wait 10 seconds for connection:
    delay(10000);
  }

  DebugMsgs.println("Connected to wifi");
  printWifiStatus();

  return 0;
}

UDP* MyNetworkHub::getUdpPort(uint32_t portNum) {
  WiFiUDP* wifiUdp = new WiFiUDP();
  wifiUdp->begin(portNum);
  return wifiUdp;
}

WiFiServer* MyNetworkHub::getTCPServer(uint32_t portNum) {
  WiFiServer* tcpServer = new WiFiServer(portNum);
  tcpServer->begin();
  return tcpServer;
}

void printWifiStatus(void) {
  // print the SSID of the network you're attached to:
  DebugMsgs.print("SSID: ").println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  DebugMsgs.print("IP Address: ").println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DebugMsgs.print("signal strength (RSSI):").print(rssi).println(" dBm");
}
