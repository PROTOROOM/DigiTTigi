#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include "epd.h"  // e-Paper driver
#include "assets.h"

// ================================= 
// Please setup for your WiFi AP
const char* ssid = "olleh_WiFi_E707";
const char* password = "0000007706";
// ================================= 


const char* host = "metakits.cc";
// online:1 , offline:0
byte mode = 0;


void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");Serial.println(ssid);  

  // SPI initialization
  pinMode(CS_PIN  , OUTPUT);
  pinMode(RST_PIN , OUTPUT);
  pinMode(DC_PIN  , OUTPUT);
  pinMode(BUSY_PIN,  INPUT);
  SPI.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // start mDNS responder
  if (MDNS.begin("digittigi")) {
    Serial.println("mDNS responder started");
  }

  showTitle();
  delay(5000);
  loadImage();
  delay(5000);
  showTitle();
  delay(5000);
  loadImage();

}

void loop(void) {

}

void showTitle(void) {
  Serial.println("Showing title");
  EPD_dispIndex = 11;
  EPD_dispInit();
  delay(100);
  Serial.println("load");
  for (int i=0; i<7; i++) {
    EPD_load(title[0][i]);
    delay(2);
  }
  EPD_SendCommand(0x13);
  delay(2);
  for (int i=0; i<7; i++) {
    EPD_load(title[1][i]);
    delay(2);
  }
  Serial.println("done");
  EPD_showB();  
}


void loadData(int i, String data) {
  if (i == 7) {
    EPD_SendCommand(0x13);
    delay(2);
  }
  EPD_load(data);
  delay(2);
}

int loadImage(void) {
  Serial.println("loading image ");

  WiFiClient client;
  const int httpPort = 80;
    
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return -1;
  }

  int key = 1; // XXX
  String url = "/load.html";
//  url += key;

  String req = String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n";
  Serial.println(req);
  client.print(req);  
    
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return -2;
    }
  }

  Serial.println("Showing new image");
  EPD_dispIndex = 11;
  EPD_dispInit();
  int dataCount = 0;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.startsWith("LOAD")) {
      line.trim();
      loadData(dataCount, line.substring(4));
      dataCount++;
      Serial.println(line.substring(4));
    }   
  }
  EPD_showB();  

  Serial.println();
  Serial.println("closing connection");
  return 0;              
}
