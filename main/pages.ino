#include <Arduino.h> 
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

//http://192.168.178.200/data?data1=2&data2=1

int mapPage = 0;
int subPage = 0;
int retakePage = 0;
int sidePage = -1;
boolean newPage = true;
boolean attack;
int buttonsOnScreen = 0;
boolean backButton = false;
boolean retake = false;

int buttonY = 250;
int buttonDist = 150;

const char* siteNumbering[] = {"A", "B", "C"};

String data1 = "-3"; //-3 unchanged m5 state
String lastData1 = "-4";
String data2 = "-3";

int buttonWidth = 300;
int buttonHeight = 100;

/*
page -1 waiting //Tbd
page 0 home
page 1 Abyss
page 2 Haven
page 3 Bind
page 4 Split
page 5 Ascent
page 6 Icebox
page 7 Breeze
page 8 Fracture
page 9 Pearl
page 10 Lotus
page 11 Sunset
page 12 Corrode
page 13 null
page 14 null


subpages:

page 0 no subpage
page 1 A
page 2 B site
page 3 C site
page 4 Mid
page 5 Retake

retakePages:

page 0 no retake page
page 1 Retake A
page 2 Retake B
page 3 Retake C

attack vs defense page:  sidePage

page 0 = attack
page 1 = defense

*/


/*void setup() {
   auto cfg = M5.config();
   M5.begin(cfg); 
   M5.Display.setRotation(2);
   M5.Display.setFont(&fonts::Font0);
   M5.Display.setEpdMode(epd_mode_t::epd_fast);
    
  wifiSetup();
  serverSetup();
  
  sdSetup();
  drawHome();


  

}*/

void setupSmokes(){
  drawHome();
  
}

/*void wifiSetup(){

   WiFi.config(local_IP, gateway, subnet);

   WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        drawNoWifi();
        delay(500); 
    }
    M5.Display.setTextSize(3);
    M5.Display.setCursor(0, 0);  
    drawWifi();

}*/
/*void sdSetup(){

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

}*/
/*void serverSetup(){

  server.on("/data", handleUpdate);
  server.begin();

}*/

void handleUpdate(){

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Received");

    String newData1 = data1; 
    String newData2 = data2;

    if (server.hasArg("data1") && server.hasArg("data2")) {
      if(server.arg("data1")!= "unknown" && server.arg("data1")!= "undefined") {
         newData1 = server.arg("data1");
      }

      if(server.arg("data2")!= "unknown" && server.arg("data2")!= "undefined") {
         newData2 = server.arg("data2");
      }
       
    }

    if (newData1 != data1 || newData2 != data2) {
        data1 = newData1;
        data2 = newData2;
        M5.Display.fillRect(120,10,300,180,TFT_WHITE);
        newPage = true;
    }

}

void smokesLoop() {

      server.handleClient();

     if (M5.Touch.getCount() > 0) {
        auto detailS = M5.Touch.getDetail(0);

        if (detailS.wasPressed()) {
          int sx = detailS.x; 
          int sy = detailS.y;   
          handleTouchSmokes(sx,sy);

        }
      } 

  if (newPage) {
          buttonsOnScreen = 0;
          drawSide();
          mapPage = data1.toInt();
          
          switch (mapPage) {
            
            case 0:
            drawHome();
            break;
            case 1:
            drawMapAbyss();
            break;
            case 2:
            drawMapHaven();
            break;
            case 3:
            drawMapBind();
            break;
            case 4:
            drawMapSplit();
            break;
            case 5:
            drawMapAscent();
            break;
            case 6:
            drawMapIcebox();
            break;
            case 7:
            drawMapBreeze();
            break;
             case 8:
            drawMapFracture();
            break;
             case 9:
            drawMapPearl();
            break;
             case 10:
            drawMapLotus();
            break;
             case 11:
            drawMapSunset();
            break;
             case 12:
            drawMapCorrode();
            break;
         

            }
        drawStatusBar();
        }

   

}

void handleTouchSmokes(int x, int y){


  for(int i = 0; i<buttonsOnScreen; i++){

    if (x >= 270 - buttonWidth / 2 && x <= 270 + buttonWidth / 2 && 
      y >= (buttonY - buttonHeight / 2) + i*buttonDist && y <= (buttonY + buttonHeight/2)+ i*buttonDist) {
            M5.Display.setCursor(0, 800);      
             buttonPressed(i);
      }


  }

  if(backButton){

    if (x >= 0 && x <= 110 && 
      y >= 850 && y <= 946) {
            goBack();
      }
  }

}

void buttonPressed(int b){
  //button b was pressed on page mapPage: buttons on screen either A B mid retake or A B C mid retake
  if(retake == false){
  if(buttonsOnScreen == 4 && subPage == 0){
    if(b < 2){
      subPage = b+1;
    }
    if(b >= 2){
      subPage = b+2;
    }

  } else if(buttonsOnScreen == 5 && subPage == 0) {

    subPage = b + 1;


  }
  }
  if(retake){
    retakePage = b+1;

  }

  newPage = true;

}

void drawButton(int x, int y, int w, int h, String label){

  M5.Display.setCursor(x, y); 
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGRAY);
  M5.Display.drawRect(x - (w/2), y - (h/2), w, h, TFT_BLACK);
  M5.Display.fillRect(x - (w/2) + 2, y - (h/2) + 2, w - 4, h - 4, TFT_LIGHTGRAY);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString(label, x, y);
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);


}

void drawMapAbyss(){ //map 1
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,180,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B  //number of sites
              break;
            case 1: //A Site
              M5.Display.fillRect(0,180,540,960,TFT_WHITE);
              switch(sidePage){

                case 0:
                abyssAttackA();
                break;

                case 1:
                abyssDefenseA();
                break;

              }
              drawBackButton();
              break;
            case 2: //B Site
            M5.Display.fillRect(0,180,540,960,TFT_WHITE); 
              switch(sidePage){

                case 0:
                abyssAttackB();
                break;

                case 1:
                abyssDefenseB();
                break;

              }
             
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,180,540,960,TFT_WHITE); 
              switch(sidePage){

                case 0:
                break;

                case 1:
                
                break;

              }
              drawBackButton();
              break;
            case 4: //Mid
            M5.Display.fillRect(0,180,540,960,TFT_WHITE); 
              switch(sidePage){

                case 0:
                abyssAttackMid();
                break;

                case 1:
                abyssDefenseMid();
                break;

              }
              drawBackButton();
              break;
            case 5: //Retake
            M5.Display.fillRect(0,180,540,960,TFT_WHITE); 
             //TBD

             switch(retakePage){

                case 0:
                //draw retake Buttons
                retake = true;
                drawButtonLayout(2);
                break;

                case 1: //A site
                if(sidePage == 0){
                  abyssPostplantA();
                }
                else if(sidePage == 1){
                  abyssRetakeA();

                }
                break;

                case 2:   //B site
                  if(sidePage == 0){  //attack = post-plant
                    abyssPostplantB();
                  }
                  else if(sidePage == 1){
                    abyssRetakeB();  //def = retake

                  }
                break;

              }
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Abyss", 270, 70);

}

void drawMapHaven(){ //map 2
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(3); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("C SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Haven", 270, 80);

}

void drawMapBind(){ //map 3
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Bind", 270, 80);

}

void drawMapSplit(){ //map 4
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Split", 270, 80);

}

void drawMapAscent(){ //map 5
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Ascent", 270, 80);

}

void drawMapIcebox(){ //map 6
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Icebox", 270, 80);

}

void drawMapBreeze(){ //map 7
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Breeze", 270, 80);

}

void drawMapFracture(){ //map 8
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Fracture", 270, 80);

}

void drawMapPearl(){ //map 9
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Pearl", 270, 80);

}

void drawMapLotus(){ //map 10
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(3); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("C SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Lotus", 270, 80);

}

void drawMapSunset(){ //map 11
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Sunset", 270, 80);

}

void drawMapCorrode(){ //map 12
  newPage = false;

  switch (subPage) {
            case 0:
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              drawButtonLayout(2); //A, B
              break;
            case 1: //A Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("A SITE", 270, 480);
              drawBackButton();
              break;
            case 2: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
              case 3: //C Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("B SITE", 270, 480);
              drawBackButton();
              break;
            case 4: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("MID", 270, 480);
              drawBackButton();
              break;
            case 5: //B Site
              M5.Display.fillRect(0,buttonY-buttonHeight/2,540,960,TFT_WHITE); 
              M5.Display.setTextDatum(middle_center);          
              M5.Display.drawString("RETAKE", 270, 480);
              drawBackButton();
              break;
            
         

            }
 

  M5.Display.setTextSize(7);
  M5.Display.setTextDatum(middle_center);          
  M5.Display.drawString("Corrode", 270, 80);

}

void drawBackButton(){
  M5.Display.fillTriangle(22,915,64,890,64,940,TFT_DARKGRAY);
  backButton = true;

}

void goBack(){
  newPage=true;
  subPage=0;
  retakePage=0;
  retake = false;
  backButton = false;
  M5.Display.fillRect(0,180,540,960,TFT_WHITE); 

}

void drawHome(){
  M5.Display.setTextSize(7);
  M5.Display.fillRect(0,70,540,960,TFT_WHITE); 
  M5.Display.setTextDatum(middle_center);   
  M5.Display.drawString("Toxic's", 270, 210);
  M5.Display.drawString("Amazing", 270, 270);
  //M5.Display.setTextSize(5);
  M5.Display.drawString("Smoke Guide", 270, 330);
  M5.Display.setTextSize(5);
   M5.Display.setEpdMode(epd_mode_t::epd_quality);         
  M5.Display.drawPngFile(SD,"/catOmen.png",84,400);
  M5.Display.setEpdMode(epd_mode_t::epd_fast);
  //M5.Display.drawString("play a match", 270, 580);
 // M5.Display.drawString("to start", 270, 620);
}

void drawWaiting(){

}

void drawSmokeImage(String name, int pos){
  int coordinate = 0;
  if(pos == 1){
    coordinate = 180;
  }
   if(pos == 2){
    coordinate = 510;
  }
  M5.Display.setTextDatum(middle_center);
  M5.Display.setEpdMode(epd_mode_t::epd_quality);         
  M5.Display.drawPngFile(SD,"/" + name + ".png",0,coordinate);
  M5.Display.setEpdMode(epd_mode_t::epd_fast);
}

void drawButtonLayout(int n){

  int cursor;

  M5.Display.setTextSize(5);
  
  for(int i=0; i<n; i++){
    drawButton(270,buttonY+i*buttonDist,buttonWidth,buttonHeight,siteNumbering[i]);
    cursor = buttonY + i*buttonDist;
    buttonsOnScreen++;
  }
  //only if retakePage == 0
  if(retake == false){
    drawButton(270,cursor+1*buttonDist,buttonWidth,buttonHeight,"Mid");
    buttonsOnScreen++;
    if(sidePage == 0){ //attack -> post plant
      drawButton(270,cursor+2*buttonDist,buttonWidth,buttonHeight,"Post-plant");
      buttonsOnScreen++;
    }
    if(sidePage == 1){ //defense -> retake
      drawButton(270,cursor+2*buttonDist,buttonWidth,buttonHeight,"Retake");
      buttonsOnScreen++;
    }
  }


}



void drawSide(){
  sidePage = data2.toInt();
  if(sidePage == 0){
    M5.Display.setTextSize(5);
    M5.Display.setTextDatum(middle_center);
    M5.Display.fillRect(0,110,540,60,TFT_WHITE);         
    M5.Display.drawString("Attack", 270, 125);
  }
  else if(sidePage == 1){
    M5.Display.setTextSize(5);
    M5.Display.setTextDatum(middle_center);    
    M5.Display.fillRect(0,110,540,60,TFT_WHITE);      
    M5.Display.drawString("Defense", 270, 125);

  }
  else{
  M5.Display.fillRect(0,110,540,60,TFT_WHITE);

  }

}
