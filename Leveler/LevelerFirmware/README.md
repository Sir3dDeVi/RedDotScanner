Leveler firmware
================

 1. Open Arduino
 2. Install NodeMCU board support:
     1. _File_ - _Preferences_
     2. Add the following URL to _Additional Boards Manager URLs_:

        http://arduino.esp8266.com/stable/package_esp8266com_index.json

     3. Click _OK_
     4. _Tools_ - _Board: &lt;something&gt;_ - _Boards Manager..._
     5. Search for `esp8266` and install _ESP8266 Community_
     6. Click _Close_
     7. In the _Tools_ - _Board: &lt;something&gt;_ - _ESP8266 Boards (3.0.2)_ menu select _NodeMCU 1.0 (ESP-12E Module)_
 3. Instal NeoPixel library for the WS2821b LEDs:
     1. _Sketch_ - _Include Library_ - _Manage Libraries..._
     2. Search for `neopixel` and install the _Adafruit NeoPixel_ library
     3. Click _Close_
