#define LOGGING
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Wire.h>
#include <InfluxDb.h>

#define I2C_ADDRESS 0x40
#define TEMPERATURE 0xE3
#define HUMIDITY 0xE5

#define DEVICE_NAME "ESP8266_PMSD001"
#define INFLUXDB_URL "http://192.168.1.128:8086"
#define INFLUX_DB_NAME "INFLUX_DB_NAME"
#define INFLUX_MEASUREMENT_NAME "MEASUREMENT_NAME"
#define NET_SSID "SSID"
#define NET_PW "password"

InfluxDBClient client(INFLUXDB_URL, INFLUX_DB_NAME);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
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

void writeToDb() {
  Point pointDevice(INFLUX_MEASUREMENT_NAME);
  pointDevice.addTag("device", DEVICE_NAME);
  pointDevice.addTag("SSID", WiFi.SSID());

  pointDevice.addField("rssi", WiFi.RSSI());
  pointDevice.addField("uptime", millis());
  pointDevice.addField("temperature", temperature());
  pointDevice.addField("humidity", humidity());

  if(!client.writePoint(pointDevice)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

int parseSensorValue(int writeVal, int bytes) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(writeVal);
  int status = Wire.endTransmission();

  if (status != 0) {
    Serial.println('ERROR: Error in transmission');
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
  int raw = parseSensorValue(TEMPERATURE, 2);
  if (raw == false) {
    return 0;
  }
  float humidity = ((125 * raw) / 65536) - 6;
  Serial.println("HUMIDITY: " + String(humidity));
  return humidity;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectToNetwork();
  }
  writeToDb();
//  delay(2000);
  delay(300000); // 5 minutes in ms
}
