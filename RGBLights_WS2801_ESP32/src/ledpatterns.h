#include <Arduino.h>

// apparently FastLED has issues with ESP32 : https://forums.adafruit.com/viewtopic.php?f=24&t=137339
// the only way to make it work was using Hardware SPI, which is NOT yet officially supported for ESP32
// see https://www.reddit.com/r/FastLED/comments/hmhlxl/hardware_spi_support_on_esp32/
// I've manually added that diff
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI
#include <FastLED.h>

#define FRAMES_PER_SECOND  30
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

template <uint8_t ledsCount>
void rainbow(CRGB (&leds)[ledsCount]) {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, ledsCount, gHue, 7);
}

template <uint8_t ledsCount>
void addGlitter(CRGB (&leds)[ledsCount], fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(ledsCount) ] += CRGB::White;
  }
}

template <uint8_t ledsCount>
void rainbowWithGlitter(CRGB (&leds)[ledsCount]) {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow(leds);
  addGlitter(leds, 80);
}

template <uint8_t ledsCount>
void confetti(CRGB (&leds)[ledsCount]) {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, ledsCount, 10);
  int pos = random16(ledsCount);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

template <uint8_t ledsCount>
void sinelon(CRGB (&leds)[ledsCount]){
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, ledsCount, 20);
  int pos = beatsin16( 13, 0, ledsCount-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

template <uint8_t ledsCount>
void bpm(CRGB (&leds)[ledsCount]) {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < ledsCount; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

template <uint8_t ledsCount>
void juggle(CRGB (&leds)[ledsCount]) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, ledsCount, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, ledsCount-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// List of patterns to cycle through.  Each is defined as a separate function below.
template <uint8_t ledsCount>
using SimplePattern = void (*)(CRGB (&)[ledsCount]);
// TODO get rid of 31
SimplePattern<31> gPatterns[]{ rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}