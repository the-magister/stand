// Compile for Adafruit HUZZAH ESP8266 80 MHz, 4M (3M SPI)
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <Streaming.h>

/* Using http://asciiflow.com to diagram

  +------+                 +------+
         |                 |        ^
         + +-C0--+ +--C2-+ +        +
                                CLEG
         + +-C1--+ +--C3-+ +        +
         |                 |        v
  +------+                 +------+
           <----+CTOP+--->

     CL=(C0,C1)         CR=(C2,C3)

*/

#define NUM_LEDS_TOP 8
#define NUM_LEDS_LEG 18
#define NUM_CONTROLLERS 4
// derive overall dimensions
#define NUM_LEDS_PER_CONTROLLER (NUM_LEDS_TOP+NUM_LEDS_LEG)
#define NUM_LEDS (NUM_CONTROLLERS*NUM_LEDS_PER_CONTROLLER)

// see pixelset.h for documention on these classes
// main container
CRGBArray<NUM_LEDS> leds;
// a container per controller
CRGBSet C0( leds(0 * NUM_LEDS_PER_CONTROLLER, 1 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C1( leds(1 * NUM_LEDS_PER_CONTROLLER, 2 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C2( leds(2 * NUM_LEDS_PER_CONTROLLER, 3 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C3( leds(3 * NUM_LEDS_PER_CONTROLLER, 4 * NUM_LEDS_PER_CONTROLLER - 1) );
// top bar set, setting up for easy mirrored animations on the top bar
CRGBSet CTOP[NUM_CONTROLLERS] = {
  C0(0, NUM_LEDS_TOP - 1),
  C1(0, NUM_LEDS_TOP - 1),
  C2(0, NUM_LEDS_TOP - 1),
  C3(0, NUM_LEDS_TOP - 1)
}; 
// "left" and "right" leg set, setting up for easy mirrored animations on the legs
CRGBSet CL[NUM_CONTROLLERS / 2] = {
  C0(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1),
  C1(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1)
};
CRGBSet CR[NUM_CONTROLLERS / 2] = {
  C2(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1),
  C3(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1)
};
CRGBSet CLEG[NUM_CONTROLLERS] = {
  C0(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1),
  C1(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1),
  C2(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1),
  C3(NUM_LEDS_TOP, NUM_LEDS_PER_CONTROLLER - 1)
};


// ESP8266: WS2811_PORTA - pins 12, 13, 14 and 15
#define PORT WS2811_PORTA

const byte targetFPS = 1;

void setup() {
  Serial.begin(115200);
  
  LEDS.addLeds<WS2811_PORTA, NUM_CONTROLLERS>(leds, NUM_LEDS_PER_CONTROLLER).setCorrection(TypicalLEDStrip);
  LEDS.setBrightness(32);
  FastLED.setMaxRefreshRate(targetFPS);

  pinMode(LED_BUILTIN, OUTPUT);

  // show the memory footprints of the containers
  Serial << F("Size of: leds=") << sizeof(leds) << endl;
  Serial << F("Size of: CTOP=") << sizeof(CTOP) << endl;
  Serial << F("Size of: CLEG=") << sizeof(CLEG) << endl;
}

void loop() {
  // 1. fade everything
  leds.fadeToBlackBy(16);

  // 2. move a pixel back and forth across the top bar
  static byte topHue = 0; // current Hue
  static int topHueDelta = 5; // Hue steps 
  static fract8 topBlur = 128; // 50% blurring
  static int topLoc = random8(CTOP[0].size()); // position
  static int dir = 1; // directionality
  if( topLoc+dir > CTOP[0].size() || topLoc+dir < 0) dir *= -1; // swap directions if needed.
  topLoc+=dir; // advance location
  topHue+=topHueDelta; // advance hue
  for( byte i=0; i<NUM_CONTROLLERS; i++) {
    CTOP[i][topLoc] = CHSV(topHue, 255, 255); // paint
    CTOP[i].blur1d(topBlur); // blur, just for visual interest
  }
  
  // 3. show a rainbow on the legs
  static byte legHue = 0;
  static int legHueDelta = 5;
  legHue++;
  CTOP[0].fill_rainbow(legHue, legHueDelta); // paint
  CTOP[1] = CTOP[0]; // mirror
  CTOP[2].fill_rainbow(legHue+128, -legHueDelta); // paint, noting we're using the other side of the wheel
  CTOP[3] = CTOP[1]; // mirror

  // 4. flag the extent of the containers
  static CHSV extentColor = CHSV(HUE_BLUE, 0, 16); // color choice.  faint white
  for( byte i=0; i<NUM_CONTROLLERS; i++) {
    CTOP[i][0] = extentColor;
    CTOP[i][CTOP[i].size()-1] = CTOP[i][0];
    CLEG[i][0] = CTOP[i][0];
    CLEG[i][CLEG[i].size()-1] = CTOP[i][0];
  } // paint

  // 5. blink with fps rate
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);

  // 6. show, delaying as-needed based on target FPS
  LEDS.show();

  EVERY_N_SECONDS( 5 ) {
    float reportedFPS = FastLED.getFPS(); 
    Serial << F("FPS reported (Hz): ") << reportedFPS << endl;
  }
}
