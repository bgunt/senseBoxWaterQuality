# senseBoxWaterQuality

The senseBox WaterQuality will be an update for the senseBox, working with Arduino Mega. 
This is the first Prototype.

##Sensors

- Atlas Scientific DO Probe & EZO Circut 	- UART
- Atlas Scientific pH Probe & EZO Circut 	- UART
- DS18B20 Temperature                    	- Digital
- DIY Conductivity                       	- Analog
- RTC RV8523                             	- I2C
- SeeedGrayOLED    

##Calibration
 
For calibration i used the tentacle-setup.ino by whitebox labs (https://www.whiteboxes.ch/tentacle/docs/) and the methods described in the datasheets by Atlas Scientific.

- pH: http://www.atlas-scientific.com/_files/_datasheets/_circuit/pH_EZO_datasheet.pdf
- DO: http://www.atlas-scientific.com/_files/_datasheets/_circuit/DO_EZO_Datasheet.pdf