# Esp8266 Deep sleep sensor

This is a simple project for ESP8266 for being able to send temperature of 3 x DS18B20 sensors to a MQTT server and entering deepsleep mode for low current consumption ( for 10minutes interval a 18650 3400mah panasonic battery can las for 3 months) 

Hw modifications : 
 - for a ESP8266-01 you need to make the following mod : https://www.tech-spy.co.uk/2019/04/enable-deep-sleep-esp-01/
 - also to be able to have a low power consumption, remove all leds from the board


SW needed for being able to compile : 

 - install arduino IDE
 - copy libraries in your Documents/Arduino folder 
 - add the following link to additional boards : https://arduino.esp8266.com/stable/package_esp8266com_index.json and install esp8266 boards revision 2.5.2
 - modify user_data_node.h with your data

After complete configuration the following payload will be sent to mqtt server, under "DeviceName" :
 - info: contains [Name, Voltage]
 - sensor: contains sensor addresses saved in memory [OnBoard, temp01, temp02]
 - data: contains current temperatures for configured sensors [Onboard, temp01, temp02]



Hope all works.
