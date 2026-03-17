#include <Arduino.h> 
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WebServer.h>


int lastSentValues[4] = {-1, -1, -1, -1};
// A simple structure to define the area of a slider


void setupVolCtrl() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(2);
    // Set E-ink to fastest mode for better slider response
    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setTextSize(7);
   M5.Display.setTextDatum(middle_center); 
   M5.Display.drawString("VolCTRL", 270, 70);

    // Initialize 1 horizontal slider (Master) and 3 vertical sliders (Apps)
     // Master
    sliders[0].init(50, 200, 60, 650, 0, 1023, 512, TFT_LIGHTGRAY);      // App 1
    sliders[1].init(175, 200, 60, 650, 0, 1023, 512, TFT_LIGHTGRAY);    // App 2
    sliders[2].init(300, 200, 60, 650, 0, 1023, 512, TFT_LIGHTGRAY);     // App 3
    sliders[3].init(425, 200, 60, 650, 0, 1023, 512, TFT_LIGHTGRAY);

    for (int i = 0; i < 4; i++) sliders[i].draw();
}

void volumeCtrlLoop() {
    M5.update();
    bool moved = false;
    

    if (M5.Touch.getCount() > 0) {
        auto touch = M5.Touch.getDetail();

        for (int i = 0; i < 4; i++) {
            if (sliders[i].handleTouch(touch)) {
                
                // ONLY update the text for the slider that actually moved
                // We map the 0-1023 value to 0-100 for the display
                int displayPercent = map(sliders[i].getValue(), 0, 1023, 0, 100);
                
                M5.Display.setTextSize(5); // Size 7 is huge, 3-4 is usually better for labels
                M5.Display.setTextColor(TFT_BLACK, TFT_WHITE); // Black text, White background clears old digits
                
                // Position the text above each slider (at y=160)
                int textX = 50 + (i * 125); 
                M5.Display.setCursor(textX, 160);
                
                // The %3d%% trick: 3 spaces for the number to prevent ghosting
                M5.Display.printf("%d%% ", displayPercent);
                
                Serial.printf("%d|%d|%d|%d\n", 
                sliders[0].getValue(), 
                sliders[1].getValue(), 
                sliders[2].getValue(), 
                sliders[3].getValue());
            }
        }

            
    }
  
    delay(10); 
}