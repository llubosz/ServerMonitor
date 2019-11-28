#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2
#define TFT_MISO 19         
#define TFT_MOSI 23           
#define TFT_CLK 18 

#define VERSION 0.01

struct SystemStatus {
  String status;
  String systemFree;
  String storageFree;
  String message;
  String raid;
};

const int numChars = 164;
char receivedChars[numChars];
boolean newData = false;
int ndx = 0;


// for soft I2C Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
 
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("System monitoring");
  tft.println("Waiting for comm...");
}


void recvWithEndMarker() {
    char endMarker = '\r';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void loop() {
    recvWithEndMarker();
    if (newData == true) {
      String payload = receivedChars;
      SystemStatus currentStatus = parse(payload);
      refresh(currentStatus);
      newData = false;
    }
}

SystemStatus parse(String payload) {
  String delimiter = ";";
  SystemStatus currentStatus;

  currentStatus.status = getValue(payload, ';', 0); 
  currentStatus.systemFree = getValue(payload, ';', 1); 
  currentStatus.storageFree = getValue(payload, ';', 2); 
  currentStatus.message = getValue(payload, ';', 3); 
  currentStatus.raid = getValue(payload, ';', 4); 

  return currentStatus;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void refresh(SystemStatus currentStatus) {
  if (currentStatus.status == "GOOD") {
    setSaneColors();
  } else {
    setInsaneColors();
  }

  tft.setTextSize(3);
  tft.setCursor(35, 10);
  tft.print("CONDITION ");
  tft.println(currentStatus.status);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 50);
  tft.println(currentStatus.storageFree);
  tft.setCursor(10, 68);
  tft.println(currentStatus.systemFree);
  tft.setTextSize(1);
  tft.setCursor(10, 96);
  tft.println("Messages:");
  tft.setCursor(10, 106);
  tft.println(currentStatus.message);
  tft.setCursor(10, 126);
  tft.println("RAID status:");
  tft.setCursor(10, 136);
  tft.println(currentStatus.raid);

  tft.setTextColor(ILI9341_GREENYELLOW);
  tft.setCursor(236, 230);
  tft.print("version: ");
  tft.print(VERSION);
}

void setSaneColors() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
}

void setInsaneColors() {
  tft.fillScreen(ILI9341_RED);
  tft.setTextColor(ILI9341_WHITE);
}
