
/* Author:	Lc
 For being able to connect to Wifi using ESP01(8266EX), use the following settings :
	ESP core : 2.5.2
	Board : Generic ESP8266
	Upload speed : 115200
	Cpu freq : 80Mhz
	Crystal Freq: 26Mhz
	Flash size : 1M(64k SPIFFS)
	Flash mode : DOUT
	Flash freq : 40Mhz
	Reset method : ck
	Debug port : Disabled 
	Debug lvl : none
	IwIP varian : v2 Lower Memory
	Vtables : Flash
	Exceptions : disabled 
	Build in Leds : 2
	Erase flash : only sketch
	Espressif FW : nonos-sdk 2.2.1(legacy)
	SSL support : all SSL ciphers

 After arduino file compiled successfully set the ESP8266 to enter boot mode and Build/Upload the application.
 After upload, unplug and plug back the esp in the debugger for being able to run the application.
 Details can be seen in the debugger.
*/

#include <ArduinoJson.hpp>

#include <OneWire.h> 
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "user_data_node.h"

#define device_mode 1
#define ONE_WIRE_BUS 2//on gpio12
/*
 * 0 - timer activated set from device wait [min]
 * 1 - deep sleep mode activated set from device wait in [min]
 */
WiFiClient espClient;
PubSubClient client(espClient);
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS);

ADC_MODE(ADC_VCC);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/

float voltage_bat, temp00, temp01, temp02;
char v_str[10], temp_str_1[10], temp_str_2[10], temp_str_3[10];
char sendData1[50], sendData2[50], sendData3[50], sendData4[50];
long temp_1, lastMsg;
int aux, retry_number, retry_wifi;
String formattedDate, create_name01, create_name02, create_name03;
char payload01[100], payload02[100], payload03[100];
char JSONmessageBuffer[150], JSONmessageBuffer2[150], JSONmessageBuffer3[150];
  
void setup() {
	//pinMode(SENSOR_POWER_PIN, OUTPUT);
	//digitalWrite(SENSOR_POWER_PIN, HIGH);
	//delay(1000);
	strcpy(payload01, device_name);
	strcat(payload01, "/info");
	strcpy(payload02, device_name);
	strcat(payload02, "/sensor");
  strcpy(payload03, device_name);
  strcat(payload03, "/data");
	aux = 1;
	Serial.begin(115200);
	sensors.begin();
	delay(100);

	setup_wifi();     
	client.setServer(mqtt_server, 1883);   
}

void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(wifi_ssid);

	WiFi.begin(wifi_ssid, wifi_password);
	retry_wifi = 0;
	while (WiFi.status() != WL_CONNECTED) {
		retry_wifi++;
		delay(500);
		Serial.print(".");
		if (retry_wifi > 40)
		{
			WiFi.disconnect();
      delay(500);
      Serial.print("WiFi error! entering deep sleep");
			ESP.deepSleep(60 * 1000 * 1000 * device_wait);
		}
	}

	Serial.println("");
	Serial.println("WiFi OK ");
	Serial.print("=> ESP8266 IP address: ");
	Serial.println(WiFi.localIP());
}

void reconnect() {
	String clientId;
	char espname[14];
	while (!client.connected()) {
		Serial.print("Connecting to MQTT broker ...");
		clientId = small_device_name;
		clientId += "-";
		clientId += String(random(0xffff), HEX);
		clientId.toCharArray(espname, 12);
		Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());
		if (client.connect(espname, mqtt_user, mqtt_password)) {
			Serial.println("OK");
		}
		else {
			retry_number++;
			Serial.print("KO, error : ");
			Serial.print(client.state());
			if (retry_number <= 4)
			{
				Serial.println(" Wait 5 secondes before to retry");
				delay(5000);
			}
			else
			{
				Serial.println(" Entering deep sleep for :" + String(device_wait) + " minutes");
				ESP.deepSleep(60 * 1000 * 1000 * device_wait);
			}
		}
	}
}

void loop() {

	if (!client.connected()) {
		retry_number = 0;
		reconnect();
	}
	client.loop();

	temp_1 = millis();

	// Send a message every minute

	switch (device_mode)
	{
	case 0:
		if ((temp_1 < 10000 * 10) && (aux == 1))
		{
			aux = 0;
			send_mqtt_data();
		}
		if (device_wait != 0)
		{
			if ((temp_1 - lastMsg) > (1000 * 60 * device_wait)) {
				send_mqtt_data();
			}
		}
		break;
	case 1:
		send_mqtt_data();
		Serial.print("entering deep sleep");
		client.disconnect();
		//digitalWrite(SENSOR_POWER_PIN, LOW);
		delay(50);
		ESP.deepSleep(60 * 1000 * 1000 * device_wait);
		break;
	default:
		break;
	}
}

void send_mqtt_data()
{
	voltage_bat = (float)ESP.getVcc() / 1024.0f;
	dtostrf(voltage_bat, 3, 1, v_str);
	snprintf(sendData1, 30, "%s", v_str);
 
	sensors.requestTemperatures();
	delay(100);
 
  temp00 = sensors.getTempC(OnBoardThermometer);
	temp01 = sensors.getTempC(FirstThermometer);
	temp02 = sensors.getTempC(SecondThermometer);
 
  dtostrf(temp00, 6, 2, temp_str_1);
  snprintf(sendData2, 30, "%s", temp_str_1);
	dtostrf(temp01, 6, 2, temp_str_2);
	snprintf(sendData3, 30, "%s", temp_str_2);
	dtostrf(temp02, 6, 2, temp_str_3);
	snprintf(sendData4, 30, "%s", temp_str_3);
  
	StaticJsonBuffer<300> JSONbuffer, JSONbuffer2, JSONbuffer3;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	JsonObject& JSONencoder2 = JSONbuffer2.createObject();
  JsonObject& JSONencoder3 = JSONbuffer3.createObject();
  
	JSONencoder["Name"] = device_name;
	JSONencoder["Voltage"] = sendData1;

  create_name01 = "";
  create_name02 = "";
  create_name03 = "";
  for (int count_1 = 0;count_1 < 8;count_1++)
    {
      create_name01 += String(OnBoardThermometer[count_1], HEX);
      create_name01 += " ";
      create_name02 += String(FirstThermometer[count_1], HEX);
      create_name02 += " ";
      create_name03 += String(SecondThermometer[count_1], HEX);
      create_name03 += " ";
    }

  JSONencoder2["OnBoard"] = create_name01;
  JSONencoder2["temp01"] = create_name02;
  JSONencoder2["temp02"] = create_name03;
 
	JSONencoder3["OnBoard"] = sendData2;
  JSONencoder3["temp01"] = sendData3;
  JSONencoder3["temp02"] = sendData4;

	JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
	JSONencoder2.printTo(JSONmessageBuffer2, sizeof(JSONmessageBuffer2));
  JSONencoder3.printTo(JSONmessageBuffer3, sizeof(JSONmessageBuffer3));
 
	Serial.print("send mqtt topic : ");
	Serial.println(payload01);
	Serial.print("JSON: ");
	Serial.println(JSONmessageBuffer);
	client.publish(payload01, JSONmessageBuffer);

	Serial.print("send mqtt topic2 : ");
	Serial.println(payload02);
	Serial.print("JSON2: ");
	Serial.println(JSONmessageBuffer2);
	client.publish(payload02, JSONmessageBuffer2);

  Serial.print("send mqtt topic3 : ");
  Serial.println(payload03);
  Serial.print("JSON3: ");
  Serial.println(JSONmessageBuffer3);
  client.publish(payload03, JSONmessageBuffer3);

	lastMsg = temp_1;
	Serial.println();   
}
