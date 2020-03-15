#define device_name  "living_room_sensor"
#define small_device_name "node00" //6characters small name

#define device_wait 5 //number of minutes between data sent intervals
#define wifi_ssid "your-network-ssid"
#define wifi_password "your-network-password"

#define mqtt_server "192.168.1.1" //mqtt server ip address or hostname
#define mqtt_user "mqttuser"      //mqtt server user if exists
#define mqtt_password "mqttpass"  //idem 

DeviceAddress InsideThermometer = { 0x28, 0x7F, 0x9B, 0x79, 0x97, 0x07, 0x03, 0x80 };   // first DS18B20 sensor address
DeviceAddress ExteriorThermometer = { 0x28, 0x5C, 0x7F, 0x79, 0x97, 0x07, 0x03, 0xAA }; // second DS18B20 sensor address