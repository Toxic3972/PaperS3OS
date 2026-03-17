#include <Arduino.h> 
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "config.h"
Preferences prefs;

#define SD_SPI_SCK_PIN  39
#define SD_SPI_MISO_PIN 40
#define SD_SPI_MOSI_PIN 38
#define SD_SPI_CS_PIN   47

IPAddress local_IP(192, 168, 178, 200);

IPAddress gateway(192, 168, 178, 1);

IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

bool launch = false;
bool homeButton = false;

struct SliderRect {
    int x, y, w, h;
    
    // Checks if a touch point is inside this slider's box
    bool contains(int touchX, int touchY) {
        return (touchX >= x && touchX < (x + w) && touchY >= y && touchY < (y + h));
    }
};

class SimpleSlider {
private:
    SliderRect area;
    int minVal, maxVal, currentVal;
    uint16_t color;
    bool isVertical;

public:
    // Setup the slider properties
    void init(int x, int y, int w, int h, int minV, int maxV, int startV, uint16_t sliderColor) {
        area = {x, y, w, h};
        minVal = minV;
        maxVal = maxV;
        currentVal = startV;
        color = sliderColor;
        isVertical = (h > w); // If height is greater than width, it's a vertical slider
    }

    // Draw the slider on the E-ink screen
    void draw() {
        M5.Display.drawRect(area.x, area.y, area.w, area.h, TFT_BLACK); // Outer frame
        updateVisuals(currentVal);
    }

    // Update logic when the screen is touched
    bool handleTouch(m5::touch_detail_t &touch) {
        if (!area.contains(touch.x, touch.y)) return false;

        // Calculate new value based on touch position
        int newValue;
        if (isVertical) {
            newValue = map(touch.y, area.y + area.h, area.y, minVal, maxVal);
        } else {
            newValue = map(touch.x, area.x, area.x + area.w, minVal, maxVal);
        }

        // Constrain the value to be within our min/max
        newValue = constrain(newValue, min(minVal, maxVal), max(minVal, maxVal));

        if (newValue != currentVal) {
            updateVisuals(newValue);
            currentVal = newValue;
            return true; // Value changed!
        }
        return false;
    }

    void updateVisuals(int val) {
        // Clear the old slider "thumb" area (simplified)
        M5.Display.fillRect(area.x + 1, area.y + 1, area.w - 2, area.h - 2, TFT_BLACK);

        // Calculate where the "thumb" (the moving indicator) should be
        int thumbSize = (isVertical) ? area.w - 4 : area.h - 4;
        int px, py;

        if (isVertical) {
            px = area.x + 2;
            py = map(val, minVal, maxVal, area.y + area.h - thumbSize - 2, area.y + 2);
        } else {
            px = map(val, minVal, maxVal, area.x + 2, area.x + area.w - thumbSize - 2);
            py = area.y + 2;
        }

        M5.Display.fillRoundRect(px, py, thumbSize, thumbSize, 3, color);
    }

    int getValue() { return currentVal; }
};

// --- Main Program ---

SimpleSlider sliders[4];
int apps = 0;
int numberOfApps = 2;

int app1x = 50;
int app1y = 153;; //without Homescreen
//main logic

/*apps:
0. Homescreen
1. Smoke Guide
2. VolCTRL

*/


void setup() {
  auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    M5.Display.setRotation(2);
    // Set E-ink to fastest mode for better slider response
    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setTextSize(7);
    wifiSetup();
    sdSetup();
    usbSetup();
    
  // put your setup code here, to run once:

}

void wifiSetup(){

   WiFi.config(local_IP, gateway, subnet);

   WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        drawNoWifi();
        delay(500); 
    }
    M5.Display.setTextSize(3);
    M5.Display.setCursor(0, 0);  
    drawWifi();

}

void usbSetup(){
  if (Serial) {
    drawUsb();
  }
  else{
    drawNoUsb();
  }
  
}
void sdSetup(){

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  M5.Display.setTextSize(3);  
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    // Print a message if SD card initialization failed or if the SD card does not exist.
    drawNoSdCard();
    while (1)
      ;
  } else {
    drawSdCard();
  }

}




void loop() {
  M5.update();

  if (M5.Touch.getCount() > 0) {
        auto detail = M5.Touch.getDetail(0);

        if (detail.wasPressed()) {
          int tx = detail.x; 
          int ty = detail.y;     
          handleTouchHome(tx,ty);

        }
      } 

  // put your main code here, to run repeatedly:
  if(apps == 0){ //Homescreen
  drawHomescreen();
  }

  if(apps == 1){ //Homescreen
  if(launch){
    setupVolCtrl();
    launch=false;
  }
  volumeCtrlLoop();
  }

   if(apps != 0){ //Homescreen
    drawHomeButton();
  }

}

void drawHomescreen(){
  drawAppIcon("black","VolCTRL",app1x,app1y);
 

}

void handleTouchHome(int x, int y){

  if(x>=app1x+25-30 && x<= app1x+25+30 && y>=app1y+25-30 && y<=app1y+25+30){
    apps = 1;
    launch = true;
  }
}

void drawAppIcon(String icon, String appName, int x, int y){
  M5.Display.fillRect(x,y,100,100,TFT_BLACK);
   M5.Display.setTextDatum(middle_center);
   M5.Display.setTextSize(4);
  M5.Display.drawString(appName, x+50, y+120);


}

void drawHomeButton(){
  homeButton = true;
  M5.Display.fillRect(249, 897, 42, 10, TFT_DARKGREY);
  M5.Display.fillRect(249, 912, 42, 10, TFT_DARKGREY);
  M5.Display.fillRect(249, 927, 42, 10, TFT_DARKGREY);

}

void drawWifi(){
  M5.Display.fillRect(478, 15, 9, 41, TFT_BLACK);
  M5.Display.fillRect(466, 24, 9, 32, TFT_BLACK);
  M5.Display.fillRect(454, 33, 9, 23, TFT_BLACK);
  M5.Display.fillRect(442, 42, 9, 14, TFT_BLACK);
}

void drawNoWifi(){
  M5.Display.fillRect(478, 15, 9, 41, TFT_DARKGRAY);
  M5.Display.fillRect(466, 24, 9, 32, TFT_DARKGRAY);
  M5.Display.fillRect(454, 33, 9, 23, TFT_DARKGRAY);
  M5.Display.fillRect(442, 42, 9, 14, TFT_DARKGRAY);
}

void drawSdCard(){
  M5.Display.fillRect(497, 15, 30, 27, TFT_BLACK);
  M5.Display.fillRect(501, 42, 26, 14, TFT_BLACK);
  M5.Display.fillRect(503, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(509, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(515, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(521, 42, 4, 12, TFT_WHITE);
 

}

void drawNoSdCard(){

  M5.Display.fillRect(497, 15, 30, 27, TFT_DARKGRAY);
  M5.Display.fillRect(501, 42, 26, 14, TFT_DARKGRAY);
  M5.Display.fillRect(503, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(509, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(515, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(521, 42, 4, 12, TFT_WHITE);
  M5.Display.fillRect(508, 20, 9, 3, TFT_WHITE);
  M5.Display.fillRect(514, 23, 3, 5, TFT_WHITE);
  M5.Display.fillRect(510, 28, 7, 3, TFT_WHITE);
  M5.Display.fillRect(510, 31, 3, 2, TFT_WHITE);
  M5.Display.fillRect(510, 35, 3, 3, TFT_WHITE);
}

void drawUsb(){
 M5.Display.fillRect(420, 45, 6, 11, TFT_BLACK);
 M5.Display.fillRect(413, 23, 20, 22, TFT_BLACK);
 M5.Display.fillRect(417, 18, 12, 5, TFT_DARKGRAY);

}

void drawNoUsb(){
 M5.Display.fillRect(420, 45, 6, 11, TFT_DARKGRAY);
 M5.Display.fillRect(413, 23, 20, 22, TFT_DARKGRAY);
 M5.Display.fillRect(417, 18, 12, 5, TFT_DARKGRAY);
  
}