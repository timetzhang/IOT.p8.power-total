#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Spark";
const char* password = "laputalpt";
String chip_id = "p8_dash_power_0001";
int bps = 1200;
ESP8266WebServer server(80);

uint8_t cur_power_code[] = { 0x68, 0x24, 0x18, 0x81, 0x00, 0xAA, 0xAA, 0x68, 0x01, 0x02, 0x63, 0xE9, 0x30, 0x16 };
uint8_t cur_power_result[18];
uint8_t total_power_code[] = { 0x68, 0x24, 0x18, 0x81, 0x00, 0xAA, 0xAA, 0x68, 0x01, 0x02, 0x43, 0xC3, 0xEA, 0x16 };
uint8_t total_power_result[18];

void UpdateIPAddress() {
  uint16_t port = 80;
  char * host = "pipa.joinp8.com";
  String IP = WiFi.localIP().toString();
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(1000);
  }
  String url = "/api/setDeviceIpAddress";
  url += "?chip=" + chip_id + "&ip=";
  url += IP;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

void setup(void) {
	Serial.begin(bps, SERIAL_8E1);
	//builtin led
	pinMode(LED_BUILTIN, OUTPUT);
	WiFi.begin(ssid, password);
	Serial.println("");
	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
		Serial.print(".");
		digitalWrite(LED_BUILTIN, HIGH);
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	if (MDNS.begin("room404")) {
		Serial.println("MDNS responder started");
	}
	digitalWrite(LED_BUILTIN, HIGH);

	server.begin();
	Serial.println("HTTP server started");

  UpdateIPAddress();
}

void sendValue(String chip_id, String device, String value)
{
  uint16_t port = 80;
  char * host = "pipa.joinp8.com";
  String IP = WiFi.localIP().toString();
  WiFiClient client;
  if (!client.connect(host, port)) {
    delay(1000);
  }
  String url = "/api";
  url += "/setdevicevalue/?chip=" + chip_id + "&device=" + device + "&value=" + value;
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

String zeroAdd(uint8_t value)
{
	return value < 0x0F ? "0" + String(value, HEX) : String(value, HEX);
}

String getCurPower() {
	//404 cur_power
	Serial.write(cur_power_code, 14);
	Serial.readBytes(cur_power_result, 18);
	String v = "0." + zeroAdd(cur_power_result[13] - 0x33) + zeroAdd(cur_power_result[12] - 0x33);
	//Serial.println(v);
	float power = v.toFloat() * 80;
	//Serial.println(power);
	return String(power);
}

String getTotalPower() {
	Serial.write(total_power_code, 14);
	Serial.readBytes(total_power_result, 18);
	String val = zeroAdd(total_power_result[15] - 0x33) + zeroAdd(total_power_result[14] - 0x33) + zeroAdd(total_power_result[13] - 0x33) + "." + zeroAdd(total_power_result[12] - 0x33);
	float result = val.toFloat() * 80;
	return String(result);
}

void loop(void) {
  if (millis()%600000 == 0)
  {
    sendValue("p8_dash_power_0001", "total_power", getTotalPower());
    delay(200);
  }
  if (millis()%5000 == 0)
  {
    sendValue("p8_dash_power_0001", "cur_power", getCurPower());
    delay(200);
  }
}
