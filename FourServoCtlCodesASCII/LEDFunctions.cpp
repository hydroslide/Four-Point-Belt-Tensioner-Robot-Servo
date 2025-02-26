// LEDFunctions.cpp
#include "LEDFunctions.h"

LEDFunctions::LEDFunctions() {}

long rgbTimeoutMs = 30000;
long msSinceLastRGB = 0;
long msSinceLastShow = 0;

//unsigned long previousTime=0;
bool useRainbow=true;

byte lr;
byte lg;
byte lb;
byte rr;
byte rg;
byte rb;

void LEDFunctions::setup() {
  delay(3000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, FAN_PIN_RIGHT>(rightLeds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, FAN_PIN_LEFT>(leftLeds, NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void LEDFunctions::LedLoop() {
  // Call the current pattern function once, updating the 'leds' array
  if (useRainbow)
    rainbow();
  else
    ShowRGB();

  long delta = TimeDelta();

  msSinceLastShow+=delta;
  // if (msSinceLastShow >= (1000/FRAMES_PER_SECOND)){
  //   msSinceLastShow=0;
  //    // send the 'leds' array out to the actual LED strip
  //   FastLED.show();
  // }


  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  msSinceLastRGB+=delta;
  if(msSinceLastRGB > rgbTimeoutMs){
    useRainbow=true;
  }
  
}

 long LEDFunctions::TimeDelta(){
  /*
  // TODO: Make this work again
  unsigned long currentTime = millis();
   long delta = (long)(currentTime-previousTime);
  previousTime = currentTime;
  return delta;
  */
 return 0;
}

void LEDFunctions::SetColorLevel(byte colorIndex, byte value){
  switch (colorIndex){
    case 0:
      lr=value;
      break;
      case 1:
      lg=value;
      break;
      case 2:
      lb=value;
      break;
      case 3:
      rr=value;
      break;
      case 4:
      rg=value;
      break;
      case 5:
      rb=value;
      break;
  }
  msSinceLastRGB=0;
}

void LEDFunctions::ShowRGB(){
  for(int i = 0; i < NUM_LEDS; i++) {
    leftLeds[i].setRGB(lr,lg,lb);
    rightLeds[i].setRGB(rr,rg,rb);
  }
  //leftLeds(0,NUM_LEDS).setRGB(lr,lg,lb);
  //rightLeds(0,NUM_LEDS).setRGB(rr,rg,rb);
}

void LEDFunctions::rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( rightLeds, NUM_LEDS, gHue, 7);
  fill_rainbow( leftLeds, NUM_LEDS, gHue, 7);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void LEDFunctions::chaseDot() {
  for(int i = 0; i < NUM_LEDS; i++) {
    // set our current dot to red, green, and blue
    rightLeds[i] = CRGB::Red;
    leftLeds[i] = CRGB::Green;
    FastLED.show();
    // clear our current dot before we move on
    rightLeds[i] = CRGB::Black;
    leftLeds[i] = CRGB::Black;
    delay(100);
  }
}
