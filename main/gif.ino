int millisCurrent;
int millisOld = 0;
int frame = 0;

void gifLoop(){

  millisCurrent = millis();
  if(millisCurrent-50>=millisOld){
    millisOld = millisCurrent;

    if(frame<9){
      frame++;
    } else {
      frame = 0;
    }
    M5.Display.setEpdMode(epd_fastest);
    M5.Display.drawPngFile(SD,String("/frame") + frame + ".png",360,780);
    
  }
}