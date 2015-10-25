# ESP8266 Sousevide - Work In progress
This is a menu driven and wifi driven Sousvide controller
Based on https://learn.adafruit.com/sous-vide-powered-by-arduino-the-sous-viduino?view=all<br>

Esp8266 Connections
![alt tag](images/Sousvide_esp.png)

Electrical input/Output
![alt tag](images/InsideTheBox.png)

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/fo02opoEktk/0.jpg)](https://www.youtube.com/watch?v=fo02opoEktk)

More details:

Screen: </br>
If il9341 (SPI) is used then:</br>
mosiPin 13 </br>
clkPin 14</br>
csPin 15</br>
dcPin 5</br>

If SSD1306 is Used then:</br>
sclPin 13</br>
sdaPin 14

DS18B20</br>
dsTempPin 5

Relay Control:</br>
relayPin 2

Rotary Pins:</br>
encoderSwitchPin 0 //push button switch</br>
encoderCLK 4</br>
encoderDT 12</br>

The rotary used is
http://www.ebay.com/itm/291347112498?_trksid=p2057872.m2749.l2649&var=590467326948&ssPageName=STRK%3AMEBIDX%3AIT
