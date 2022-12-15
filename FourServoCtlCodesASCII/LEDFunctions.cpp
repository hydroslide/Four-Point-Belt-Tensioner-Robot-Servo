// LEDFunctions.cpp
#include "LEDFunctions.h"

LEDFunctions::LEDFunctions() {}

void LEDFunctions::setup() {
  delay(3000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, FAN_PIN_RIGHT>(rightLeds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, FAN_PIN_LEFT>(leftLeds, NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void LEDFunctions::loop() {
  // Call the current pattern function once, updating the 'leds' array
  rainbow();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void LEDFunctions::rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( rightLeds, NUM_LEDS, gHue, 7);
  fill_rainbow( leftLeds, NUM_LEDS, gHue, 7);
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
