
#include "epd.h"  // e-Paper driver
#include "assets.h"

void setup(void) {
  Serial.begin(115200);

  // SPI initialization
  pinMode(CS_PIN  , OUTPUT);
  pinMode(RST_PIN , OUTPUT);
  pinMode(DC_PIN  , OUTPUT);
  pinMode(BUSY_PIN,  INPUT);
  SPI.begin();

  Serial.println("start");
  delay(2000);
  EPD_dispIndex = 11;
  EPD_dispInit();
  delay(1000);
  Serial.println("load");
  for (int i=0; i<7; i++) {
    EPD_load(title[0][i]);
    delay(10);
  }
  EPD_SendCommand(0x13);
  delay(2);
  for (int i=0; i<7; i++) {
    EPD_load(title[1][i]);
    delay(10);
  }
  Serial.println("done");
  EPD_showB();

}


void loop(void) {

}
