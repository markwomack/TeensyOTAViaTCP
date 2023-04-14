# TeensyOTAViaTCP

This is a working example of updating the firmware of a Teensy Over-The-Air (aka OTA) via a TCP connection. The Teensy is connected to a WiFi network using the Arduino WiFiNINA library, and uses the [FlasherX library](https://github.com/joepasquariello/FlasherX) to perform the actual firmware update.

In this specific code I am using the [Adafruit AirLift Breakout module](https://www.adafruit.com/product/4201) to connect to WiFi, but theoretically any WiFi capable chip/breakout should work, so long as it is compatible with the WiFiNINA library ([Adafruit's WiFiNina version is required](https://github.com/adafruit/WiFiNINA)). If you need to use different code to connect to WiFi, then YMMV.

This is currently a work in progress.
