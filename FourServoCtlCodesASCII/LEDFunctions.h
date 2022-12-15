// LEDFunctions.h
#ifndef LED_FUNCTIONS_H
#define LED_FUNCTIONS_H

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define FAN_PIN_RIGHT    9
#define FAN_PIN_LEFT    3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    16

#define BRIGHTNESS          100
#define FRAMES_PER_SECOND  120

class LEDFunctions {
public:
  LEDFunctions();
  void setup();
  void loop();

private:
  CRGB leds[NUM_LEDS];
  CRGB rightLeds[NUM_LEDS];
  CRGB leftLeds[NUM_LEDS];
  uint8_t gHue = 0;
  
  void rainbow();
  void chaseDot();
};

#endif // LED_FUNCTIONS_H
