#define LOGGING
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Wire.h>
#include <PubSubClient.h>

#define I2C_ADDRESS 0x40
#define TEMPERATURE 0xE3
#define HUMIDITY 0xE5

#define DEVICE_NAME "ESP8266_PMSD001"
#define NET_SSID "SSID"
#define NET_PW "password"

#define MQTT_DOMAIN "192.168.1.38"
#define MQTT_PORT 1883

#define TOPIC "house/room/office"
#define MEASUREMENT "climate"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
}

int parseSensorValue(int writeVal, int bytes) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(writeVal);
  int status = Wire.endTransmission();

  if (status != 0) {
    Serial.println("ERROR: Error in transmission");
    return false;
  }

  Wire.requestFrom(I2C_ADDRESS, bytes);

  int response[bytes];
  int index = 0;

  while(Wire.available()) {
    byte b = Wire.read();
    response[index++] = b;
  }

  //  convert to int
  return response[0]*256 | response[1];
}

void reconnectToNetwork() {
  Serial.print("(Re)Connecting to ");
  Serial.println(NET_SSID);
  WiFi.begin(NET_SSID, NET_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

float temperature() {
  int raw = parseSensorValue(TEMPERATURE, 2);
  if (raw == false) {
    return 0;
  }
  float temp = ((175.72 * raw) / 65536) - 46.85;

  Serial.println("TEMP: " + String(temp));
  return temp;
}

float humidity() {
  int raw = parseSensorValue(HUMIDITY, 2);
  if (raw == false) {
    return 0;
  }
  float humidity = ((125 * raw) / 65536) - 6;

  Serial.println("HUMIDITY: " + String(humidity));
  return humidity;
}

String line_protcol_temp() {
  return "temp=" + String(temperature());
}

String line_protcol_humi() {
  return "humi=" + String(humidity());
}

String line_protocol() {
  return String(MEASUREMENT) + " " + line_protcol_humi() + "," + line_protcol_temp();
}

void publish() {
  client.setServer(MQTT_DOMAIN, MQTT_PORT);
  if(!client.connect("ESP8266Client")) {
    Serial.println(client.state());
    return;
  }

  Serial.println("connected");

  client.publish(TOPIC, line_protocol().c_str());
}

void loop() {
  Serial.println("hello world!");
  if (WiFi.status() != WL_CONNECTED) {
    reconnectToNetwork();
  }

  publish();
  // delay(2000);
  delay(300000); // 5 minutes in ms
}
