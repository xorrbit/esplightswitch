#include <ESP8266WiFi.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

#define WEMO_IP "192.168.1.12"
#define WEMO_PORT 49153
#define CLK 2

void setup() {
  // spam serial port with some debug stuff
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
}

void loop() {
  int state = 0;
  int pos = -1;
  int pos_end = -1;
  unsigned long timeout;
  
  WiFiClient client;
  if (!client.connect(WEMO_IP, WEMO_PORT)) {
    Serial.println("connection failed");
    return;
  }

  String request1 =
    String("POST /upnp/control/basicevent1 HTTP/1.1\r\n") +
    "Host: 192.168.1.12:49153\r\n" +
    "SOAPACTION: \"urn:Belkin:service:basicevent:1#GetBinaryState\"\r\n" +
    "Content-Type: text/xml\r\n" +
    "Content-Length: 271\r\n" +
    "\r\n" +
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"></u:GetBinaryState></s:Body></s:Envelope>";

  Serial.println("seding request...");
  client.print(request1);
  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\r');
    pos = line.indexOf("<BinaryState>");
    if (pos != -1) {
      pos = pos + 13;
      pos_end = pos + 1;
      state = line.substring(pos, pos_end).toInt();
      client.stop();
      break;
    }
  }

  Serial.println("\nlight switch state is ");
  Serial.println(state);

  state = state ^ 1; // toggle

  Serial.println("toggled state is:");
  Serial.println(state);

  
  if (!client.connect(WEMO_IP, WEMO_PORT)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("new state string is");
  Serial.println(String(state));
  String request2 =
    String("POST /upnp/control/basicevent1 HTTP/1.1\r\n") +
    "Host: 192.168.1.12:49153\r\n" +
    "SOAPACTION: \"urn:Belkin:service:basicevent:1#SetBinaryState\"\r\n" +
    "Content-Type: text/xml\r\n" +
    "Connection: Close\r\n" +
    "Content-Length: 299\r\n" +
    "\r\n" +
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + String(state) + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>";
    
  Serial.println("sending request2...");
  client.print(request2);
  timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
  }

  client.stop();

  // wait 10 seconds
  delay(5000);
}
