/*

 Author:	Lce
*/

#include <ArduinoJson.hpp>

#include <OneWire.h> 
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "user_data_node.h"

#define device_mode 1
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 2//on gpio12
//#define SENSOR_POWER_PIN 0//on gpio12
/*
 * 0 - timer activated set from device wait [min]
 * 1 - deep sleep mode activated set from device wait in [min]
 */


 // Create abjects

WiFiClient espClient;
PubSubClient client(espClient);
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS);

ADC_MODE(ADC_VCC);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/

float voltage_bat, tempInt, tempExt;
char v_str[10], temp_str_1[10], temp_str_2[10];
char sms_data1[50], sms_data2[50], sms_data3[50];
long temp_1, lastMsg;
int aux, retry_number;
String formattedDate, create_name;
char sensor_tele2[100], sensor_tele[100], state_tele[100];
void setup() {
	//pinMode(SENSOR_POWER_PIN, OUTPUT);
	//digitalWrite(SENSOR_POWER_PIN, HIGH);
	//delay(1000);
	strcpy(sensor_tele, device_name);
	strcat(sensor_tele, "/tele/sensor");
	strcpy(state_tele, device_name);
	strcat(state_tele, "/tele/state");
	aux = 1;
	Serial.begin(115200);
	sensors.begin();
	delay(100);
	//set resolution to 10bit.
	//sensors.setResolution(ExteriorThermometer, 10);
	//sensors.setResolution(InsideThermometer, 10);
	//sensors.setResolution(ExteriorThermometer, 9);
	//sensors.setResolution(InsideThermometer, 9);
	//delay(100);
	setup_wifi();           //Connect to Wifi network
	client.setServer(mqtt_server, 1883);    // Configure MQTT connexion
		   // callback function to execute when a MQTT message   
}

//Connexion au r�seau WiFi
void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(wifi_ssid);

	WiFi.begin(wifi_ssid, wifi_password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi OK ");
	Serial.print("=> ESP8266 IP address: ");
	Serial.println(WiFi.localIP());
}

//Reconnexion
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
	snprintf(sms_data1, 30, "%s", v_str);
	sensors.requestTemperatures();
	delay(100);
	tempInt = sensors.getTempC(InsideThermometer);
	tempExt = sensors.getTempC(ExteriorThermometer);
	dtostrf(tempInt, 6, 2, temp_str_1);
	snprintf(sms_data2, 30, "%s", temp_str_1);
	dtostrf(tempExt, 6, 2, temp_str_2);
	snprintf(sms_data3, 30, "%s", temp_str_2);
	StaticJsonBuffer<300> JSONbuffer, JSONbuffer2;
	JsonObject& JSONencoder = JSONbuffer.createObject();
	JsonObject& JSONencoder2 = JSONbuffer2.createObject();
	JSONencoder["Name"] = device_name;
	JSONencoder["Voltage"] = sms_data1;
	if ((tempInt != -127) && (tempInt != 25))
		JSONencoder["In"] = sms_data2;
	else
	{
		create_name = "";
		for (int count_1 = 0;count_1 < 8;count_1++)
		{
			create_name += String(InsideThermometer[count_1], HEX);
			create_name += " ";
		}
		JSONencoder2["In"] = create_name;
		create_name = "";
	}
	if ((tempExt != -127) && (tempInt != 25))
		JSONencoder["Out"] = sms_data3;
	else
	{
		create_name = "";
		for (int count_1 = 0;count_1 < 8;count_1++)
		{
			create_name += String(ExteriorThermometer[count_1], HEX);
			create_name += " ";
		}
		JSONencoder2["Out"] = create_name;
		create_name = "";
	}
	char JSONmessageBuffer[150];
	char JSONmessageBuffer2[150];
	JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
	JSONencoder2.printTo(JSONmessageBuffer2, sizeof(JSONmessageBuffer2));
	Serial.print("send mqtt topic : ");
	Serial.println(sensor_tele);
	Serial.print("JSON: ");
	Serial.println(JSONmessageBuffer);
	client.publish(sensor_tele, JSONmessageBuffer);

	strcpy(sensor_tele2, sensor_tele);
	strcat(sensor_tele2, "info");
	Serial.print("send mqtt topic2 : ");
	Serial.println(sensor_tele2);
	Serial.print("JSON2: ");
	Serial.println(JSONmessageBuffer2);
	client.publish(sensor_tele2, JSONmessageBuffer2);

	lastMsg = temp_1;
	//Serial.println("send mqtt");
	//Serial.println(sms_data1);
	Serial.println();
	//client.publish(sensor_tele, "1",true);   // Publish temperature on temperature_topic
	//client.publish(state_tele,sms_data1, true);    
}