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

- I use a couple of my own libraries for printing debug messages ([ArduinoLogging](https://github.com/markwomack/ArduinoLogging)) and running 'normal operations' ([TaskManager](https://github.com/markwomack/TaskManager)). You can add those libraries to your Arduino environment or just remove/replace the code.
- This code is written to use the [Adafruit AirLift co-processor breakout](https://www.adafruit.com/product/4201) to connect to WiFi, but just about any ESP32 based board will work. Please see the pin_assignments.h file for the expected pin connections in the circuit.<br/>
**IMPORTANT NOTE**: The AirLift board is required to be powered by 5v power, not 3.3v, for this example. When using 3.3v multiple restarts somehow put the AirLift (and other ESP32 based boards) into a bad state where it just continuously causes a restart when reconecting to WiFi. YMMV, but that has been my experience.
- This code is Teensy specific. See the FlasherX documentation for information on the specific Teensy models it supports. I have tested it on a Teensy 4.0 and plan to use it on a Teensy 4.1.
- I included the FlasherX code directly in this example since it it is not currently pacakged as a library. You may want to check the FlasherX github for the latest version. I made changes in the original FlasherX code to:
  - Integrate usage of the CRCStream class into the update_firmware method.
  - Comment out the code in the update_firmware method that asked for user confirmation before performing the update.
  - Modified the parse_ascii_file method to detect data that has both a carraige return and a line feed at the end of each line, and then parse accordingly.
- You'll need to know where the new, compiled code is located so that you can send it to the Teensy. Fortunately the Teensy Loader has a File->Open Hex File command that opens to the directory where the new .hex file is stored after compilation. This is typically a temporary, ephemeral location that will be cleaned up after the sketch or the Arduino IDE is closed. You can copy the .hex file to a different location if you want.
- To send the new .hex file to the Teensy, you will need to send it to the TCP port the Teensy is monitoring. However, the .hex file needs a small modfication to prepend the .hex file size and CRC code to the beginning of the file. The modified .hex file can then be sent to the TCP port of the Teensy. The [script/tcpsend.sh](https://github.com/markwomack/TeensyOTAViaTCP/blob/main/scripts/tcpSend.sh) can be used to do all of this. The call looks like:<br/>
<code>./scripts/tcpSend.sh [IP address of Teensy] [TCP port of Teensy] [path to TeensyOTAViaTCP.ino.hex file]</code><br/>
For this example, it might look like this:<br/>
<code>./scripts/tcpSend.sh 192.168.90.120 50000 /tmp/arduino_build_123456/TeensyOTAViaTCP.ino.hex</code><br/>
Running this command will send the data to the Teensy via the TCP
port, the Teensy will see it and start the update process. It will use the size and CRC code to verify that the data was transmitted without errors.

Caveats:
- In addition to whatever the TCP layer provides, this version provides a size and CRC check to guard against dropped or mangled data. You may want to introduce other
measures to ensure that the data is correctly received. Otherwise you run the risk of bricking your Teensy in-the-wild.
- The CRC code does not provide any privacy or security to the data being sent. Everyone can still see the data on the network. The CRC code only ensures that the data was probably
sent correctly. It does not obscure or encode the data at all.
- Similarly, there are no security protocols used in this example. Anyone on your network can access the TCP port and push any data they want to it. In the real world you will want to add some appropriate security to prevent unwanted access.
