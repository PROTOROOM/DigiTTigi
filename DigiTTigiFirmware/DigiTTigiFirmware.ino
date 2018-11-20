#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <EEPROM.h>
#include "epd.h"  // e-Paper driver
#include "assets.h"

// =================================
// Please setup for your WiFi AP
const char* ssid = "olleh_WiFi_E707";
const char* password = "0000007706";
//const char* ssid = "workspace";
//const char* password = "work7531";
// =================================


// book for digittigi
// book0 : for testing
// alice : Alice's Adventures in Wonderland
String bookName = "alice";


const char* host = "ttigi.protoroom.kr";
//const char* host = "protoroom.github.io";


byte pageMode = 0; // title:0, loadImage:1
byte tPage = 1; // EEPROM address 0
byte cPage = 0;
byte pageCount = 0;
byte pagePerChapter = 1; // show title after showing 3 pages


Ticker tick;
int tickTime = 60;
boolean runTick = false;


// =================================

void showTitle(void) {
  Serial.println("Showing title");
  EPD_dispIndex = 0;
  EPD_dispInit();
  delay(100);
  Serial.println("load");
  for (int i = 0; i < 7; i++) {
    EPD_load(title[0][i]);
    delay(2);
  }
  EPD_SendCommand(0x13);
  delay(2);
  for (int i = 0; i < 7; i++) {
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

void loadImage(int pageNumber) {
  Serial.println("loading image ");

  WiFiClient client;
  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String page = String(pageNumber) + ".html";
  String url = "/books/"+ bookName + "/data/" + page;

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
      return;
    }
  }

  Serial.println("Showing new image");
  EPD_dispIndex = 0;
  EPD_dispInit();
  int dataCount = 0;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.startsWith("TOTAL")) {
      line.trim();
      tPage = line.substring(5).toInt();
      EEPROM.write(0, tPage);
      EEPROM.commit();
      Serial.println("Update Total Page Number");
    }
    if (line.startsWith("LOAD")) {
      line.trim();
      loadData(dataCount, line.substring(4));
      dataCount++;
      Serial.print("loading page from : ");Serial.print(host);Serial.println(url);
//      Serial.println(line.substring(4));
    }
  }
  Serial.print("Total Page: ");Serial.println(tPage);
  Serial.print("Current Page: ");Serial.println(cPage);
  Serial.println("Loading Page Done");
  EPD_showB();
  return;
}


void updateImage(void) {
  if (!runTick) runTick = true;
}


void setup(void) {
  Serial.begin(115200);
  EEPROM.begin(512);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to "); Serial.println(ssid);

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

  tPage = EEPROM.read(0);
  Serial.print("Total Pages: "); Serial.println(tPage);

  showTitle(); pageMode = 1;
  tick.attach(tickTime, updateImage);

}


void loop(void) {
  if (runTick) {
    if (pageMode == 0) {
      showTitle();
      pageMode = 1;
    } else if (pageMode == 1) {
      cPage++;
      if (cPage > tPage) cPage = 1;
      loadImage(cPage);
      pageCount++;

      if (pageCount >= pagePerChapter) {
        pageCount = 0;
        pageMode = 0;
      }
    }
    
    runTick = false;
  }
}
