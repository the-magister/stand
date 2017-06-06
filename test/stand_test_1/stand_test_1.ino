// Compile for Adafruit HUZZAH ESP8266 80 MHz, 4M
#include<FastLED.h>

#define NUM_LEDS_PER_STRIP 38
// Note: this can be 12 if you're using a teensy 3 and don't mind soldering the pads on the back
#define NUM_STRIPS 4
#define NUM_LEDS NUM_STRIPS * NUM_LEDS_PER_STRIP
CRGB leds[NUM_LEDS];

// ESP8266: WS2811_PORTA - pins 12, 13, 14 and 15
#define PORT WS2811_PORTA

void setup() {
  // LEDS.addLeds<WS2811_PORTA,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  // LEDS.addLeds<WS2811_PORTB,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  // LEDS.addLeds<WS2811_PORTD,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  LEDS.addLeds<WS2811_PORTA,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  LEDS.setBrightness(32);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    }
  }

  // Set the first n leds on each strip to show which strip it is
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j <= i; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
    }
  }

  hue++;

  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
  
  LEDS.show();
  LEDS.delay(10);
}
