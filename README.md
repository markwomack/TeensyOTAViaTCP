# TeensyOTAViaTCP

This is a verbose example of updating a Teensy's firmware via TCP using the FlasherX code from Joe Pasquariello ([https://github.com/joepasquariello/FlasherX](https://github.com/joepasquariello/FlasherX)).

This code will:

- Connect to a WiFi network.
- Open a server on a TCP port.
- Start normal operations which will periodically check the TCP port for available data.
- When data is detected on the TCP port, normal operations are suspended, and the data is read from the TCP port.
- The data is processed and stored in flash memory.
- If the data appears to be correct, then it is copied into the Teensy firmware and the Teensy is restarted.
- After the Teensy restarts it should be running the new program.

All of the heavy lifting and updating is done by the FlasherX code. The rest of this code is just making sure the new data gets transported to the right place so FlasherX can take over.

Some notes on the implementation and usage:

- I use a couple of my own libraries for printing debug messages ([ArduinoLoggin](https://github.com/markwomack/ArduinoLogging)) and running 'normal operations' ([TaskManager](https://github.com/markwomack/TaskManager)). You can add those libraries to your Arduino environment or just remove/replace the code.
- This code is written to use the [Adafruit AirLift co-processor breakout](https://www.adafruit.com/product/4201) to connect to WiFi, but just about any ESP32 based board will work. Please see the pin_assignments.h file for the expected pin connections in the circuit.<br/>
**IMPORTANT NOTE**: The AirLift board is required to be powered by 5v power, not 3.3v, for this example. Multiple restarts somehow put the AirLift (and other ESP32 based boards) into a bad state where it just continuously causes a restart when reconecting to WiFi. YMMV, but that has been my experience.
- This code is Teensy specific. See the FlasherX documentation for information on the specific Teensy models it supports. I have tested it on a Teensy 4.0 and plan to use it on a Teensy 4.1.
- I included the FlasherX code directly in this example since it it is not currently pacakged as a library. You may want to check the FlasherX github for the latest version. The only modification I had to make on the FlasherX code was to comment out the code in the update_firmware method that asked for user confirmation before performing the update. Otherwise everything is the original code.
- You'll need to know where the new, compiled code is located so that you can send it to the Teensy. Fortunately the Teensy Loader has a File->Open Hex File command that opens to the directory where the new .hex file is stored after compilation. This is typically a temporary, ephemeral location that will be cleaned up after the sketch or the Arduino IDE is closed. You can copy the .hex file to a different location if you want.
- To send the new .hex file to the Teensy, you will need to send it to the TCP port the Teensy is monitoring. For Linux you can use the command line to cd into the directory that contains the .hex file and then execute the following command:<br/>
<code>netcat -N [IP ADDRESS OF TEENSY] [TCP PORT OF TEENSY] <[HEX FILE NAME]</code><br/>
For this example, it might look like this:<br/>
<code>netcat -N 192.168.86.123 50005 <TeensyOTAViaTCP.ino.hex</code><br/>
Running this command will send the data to the Teensy via the TCP
port, the Teensy will see it and start the update process.

Caveats:
- Besides the error detection provided by the underlying TCP protocol and the FlasherX code, there are no other protections in place for this example. If you are using this to build a high reliability, fault tolerant system it may behoove you to build in other protections and checks, such as, but not limited to, a CRC check. Otherwise you run the risk of bricking your Teensy in-the-wild.
- Similarly, there are no security protocols used in this example. Anyone on your network can access the TCP port and push any data they want to it. In the real world you will want to add some appropriate security to prevent unwanted access.
