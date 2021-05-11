#define device_name  "your-device-name"
#define small_device_name "node00" //6characters small name

#define device_wait 5 //number of minutes between data sent intervals
#define wifi_ssid "your-network-ssid"
#define wifi_password "your-network-password"

#define mqtt_server "your-mqtt-server-ip" //mqtt server ip address or hostname
#define mqtt_user "mqttuser"      //mqtt server user if exists
#define mqtt_password "mqttpass"  //idem 

DeviceAddress OnBoardThermometer = { 0x28, 0x7F, 0x9B, 0x79, 0x97, 0x07, 0x03, 0x80 }; //On board DS18B20 sensor address
DeviceAddress FirstThermometer = { 0x28, 0x7F, 0x9B, 0x79, 0x97, 0x07, 0x03, 0x80 };   // first DS18B20 sensor address
DeviceAddress SecondThermometer = { 0x28, 0x5C, 0x7F, 0x79, 0x97, 0x07, 0x03, 0xAA }; // second DS18B20 sensor address
