#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define FAN_PIN_RIGHT    9
#define FAN_PIN_LEFT    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    16
CRGB leds[NUM_LEDS];
CRGB rightLeds[NUM_LEDS];
CRGB leftLeds[NUM_LEDS];

#define BRIGHTNESS          100
#define FRAMES_PER_SECOND  120

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, FAN_PIN_RIGHT>(rightLeds, NUM_LEDS);
 FastLED.addLeds<LED_TYPE, FAN_PIN_LEFT>(leftLeds, NUM_LEDS);

  //FastLED.addLeds<LED_TYPE,FAN_PIN_RIGHT,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  //FastLED.addLeds<LED_TYPE,FAN_PIN_RIGHT,COLOR_ORDER>(rightLeds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,FAN_PIN_LEFT,COLOR_ORDER>(leftLeds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  //chaseDot();
  
  // Call the current pattern function once, updating the 'leds' array
  rainbow();
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  
}
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( rightLeds, NUM_LEDS, gHue, 7);
  fill_rainbow( leftLeds, NUM_LEDS, gHue, 7);
}

void chaseDot(){
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
