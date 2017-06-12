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
#define NUM_LEDS_LEG 15
#define NUM_CONTROLLERS 4
// derive overall dimensions
#define NUM_LEDS_PER_CONTROLLER (NUM_LEDS_TOP+NUM_LEDS_LEG)
#define NUM_LEDS (NUM_CONTROLLERS*NUM_LEDS_PER_CONTROLLER)

// use parallel output on PORTA
// ESP8266: WS2811_PORTA - pins 12, 13, 14 and 15
#define PORT WS2811_PORTA

// target FPS.  set this low to see what's going on.
const byte targetFPS = 20;

// maximum brightness
const byte masterBrightness = 255;

// see pixelset.h for documention on these classes
// main container
CRGBArray<NUM_LEDS> leds;
// a container per controller
CRGBSet C0( leds(0 * NUM_LEDS_PER_CONTROLLER, 1 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C1( leds(1 * NUM_LEDS_PER_CONTROLLER, 2 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C2( leds(2 * NUM_LEDS_PER_CONTROLLER, 3 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C3( leds(3 * NUM_LEDS_PER_CONTROLLER, 4 * NUM_LEDS_PER_CONTROLLER - 1) );
// and reversed version
CRGBSet C0r(C0, -NUM_LEDS_PER_CONTROLLER);
CRGBSet C1r(C1, -NUM_LEDS_PER_CONTROLLER);
CRGBSet C2r(C2, -NUM_LEDS_PER_CONTROLLER);
CRGBSet C3r(C3, -NUM_LEDS_PER_CONTROLLER);
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
// more exotic containers
// define a container that wraps around at the ends of each leg, creating a circuit/track
// note that we instantiate the set with the pointer
CRGB *pctrack[NUM_LEDS];
//CRGBSet CTRACK(pctrack[0], NUM_LEDS);
// and the reverse
//CRGBSet CTRACKr(CTRACK, -NUM_LEDS);
// function to set references
void setCTRACK() {
  for ( word i = 0; i < NUM_LEDS_PER_CONTROLLER; i++) {
    pctrack[0 * NUM_LEDS_PER_CONTROLLER + i] = & C0[i];
    pctrack[1 * NUM_LEDS_PER_CONTROLLER + i] = & C1r[i];
    pctrack[2 * NUM_LEDS_PER_CONTROLLER + i] = & C3[i];
    pctrack[3 * NUM_LEDS_PER_CONTROLLER + i] = & C2r[i];
  }

  // so, we've aligned the pointers to correctly address the primary storage,
  // handling the jumping around in the index.
}
// define three "little tracks". one each for looping around each leg, and one for looping around the top
CRGB *pctrackl[NUM_LEDS_LEG*2];
CRGB *pctrackr[NUM_LEDS_LEG*2];
CRGB *pctrackt[NUM_LEDS_TOP*4];
CRGBSet CTRACKL(pctrackl[0], NUM_LEDS_LEG*2);
CRGBSet CTRACKR(pctrackr[0], NUM_LEDS_LEG*2);
CRGBSet CTRACKT(pctrackt[0], NUM_LEDS_TOP*4);
// and the reverse
CRGBSet CTRACKLr(CTRACKL, -NUM_LEDS_LEG*2);
CRGBSet CTRACKRr(CTRACKR, -NUM_LEDS_LEG*2);
CRGBSet CTRACKTr(CTRACKT, -NUM_LEDS_TOP*4);
// functions to set references
void setCTRACKL() {
  for ( word i = 0; i < NUM_LEDS_LEG; i++) {
    pctrackl[0 * NUM_LEDS_LEG + i] = & CL[1][i];
    pctrackl[1 * NUM_LEDS_LEG + i] = & CL[2][NUM_LEDS_LEG-1-i];
  }
}
void setCTRACKR() {
  for ( word i = 0; i < NUM_LEDS_LEG; i++) {
    pctrackr[0 * NUM_LEDS_LEG + i] = & CR[1][i];
    pctrackr[1 * NUM_LEDS_LEG + i] = & CR[2][NUM_LEDS_LEG-1-i];
  }
}
void setCTRACKT() {
  for ( word i = 0; i < NUM_LEDS_TOP; i++) {
    pctrackt[0 * NUM_LEDS_TOP + i] = & CTOP[1][i];
    pctrackt[1 * NUM_LEDS_TOP + i] = & CTOP[2][NUM_LEDS_TOP-1-i];
    pctrackt[2 * NUM_LEDS_TOP + i] = & CTOP[3][i];
    pctrackt[3 * NUM_LEDS_TOP + i] = & CTOP[4][NUM_LEDS_TOP-1-i];
  }
}

void setup() {
  Serial.begin(115200);

  LEDS.addLeds<WS2811_PORTA, NUM_CONTROLLERS>(leds, NUM_LEDS_PER_CONTROLLER).setCorrection(TypicalLEDStrip);
  LEDS.setBrightness(masterBrightness);
  FastLED.setMaxRefreshRate(targetFPS);

  pinMode(LED_BUILTIN, OUTPUT);

  // show the memory footprints of the containers
  Serial << F("Size of: leds=") << sizeof(leds) << endl;
  Serial << F("Size of: CTOP=") << sizeof(CTOP) << endl;
  Serial << F("Size of: CLEG=") << sizeof(CLEG) << endl;

  // set up pointers to ease containers
  setCTRACK();
  setCTRACKL();
  setCTRACKR();
  setCTRACKT();
  Serial << F("Setup. complete.") << endl;
  delay(1000);
  
}

// maybe not execute everything all at once
#define RUN_TOP 0
#define RUN_LEGS 0
#define RUN_EXTENT 0
#define RUN_TRACK 1

void loop() {
  // 1. fade everything
  leds.fadeToBlackBy(16);

  if ( RUN_TOP ) {
    // 2. move a pixel back and forth across the top bar
    static byte topHue = 0; // current Hue
    static int topHueDelta = 5; // Hue steps
    static fract8 topBlur = 128; // 50% blurring
    static int topLoc = random8(CTOP[0].size()); // position. NOTE: NOT using #define's, as I want to work within a container
    static int dir = 1; // directionality
    if ( topLoc + dir > CTOP[0].size()-1 || topLoc + dir < 0) dir *= -1; // swap directions if needed.
    topLoc += dir; // advance location
    topHue += topHueDelta; // advance hue
    for ( byte i = 0; i < NUM_CONTROLLERS; i++) {
      CTOP[i][topLoc] += CHSV(topHue, 255, 255); // paint
      CTOP[i].blur1d(topBlur); // blur, just for visual interest
    }
  }

  if ( RUN_LEGS ) {
    // 3. show a rainbow on the legs
    static byte legHue = 0;
    static int legHueDelta = 5;
    legHue++;
    CLEG[0].fill_rainbow(legHue, legHueDelta); // paint
    CLEG[1] = CLEG[0]; // mirror
    CLEG[2].fill_rainbow(legHue + 128, -legHueDelta); // paint, noting we're using the other side of the wheel
    CLEG[3] = CLEG[2]; // mirror
  }

  if ( RUN_EXTENT ) {
    // 4. flag the extent of the containers
    static CHSV extentColor = CHSV(HUE_BLUE, 0, 16); // color choice.  faint white.
    for ( byte i = 0; i < NUM_CONTROLLERS; i++) {
      CTOP[i][0] += extentColor;
      CTOP[i][CTOP[i].size() - 1] += extentColor;
      CLEG[i][0] += extentColor;
      CLEG[i][CLEG[i].size() - 1] += extentColor;
    } // paint
  }

  if ( RUN_TRACK ) {
    // 5. send a bright white dot around the big track
    static CRGBSet CTRACK(pctrack[0], NUM_LEDS);
    static CHSV trackColor = CHSV(HUE_BLUE, 0, 255); // color choice.  bright white.
    static word trackPos = random8(CTRACK.size()); // current position
    trackPos++; // increment    
    if ( trackPos > CTRACK.size() - 1 ) trackPos = 0; // wrap around
    CTRACK[trackPos] += trackColor; // paint
  }

  // 6. blink with fps rate
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);

  // 7. show, delaying as-needed based on target FPS
  LEDS.show();

  // 8. show our frame rate, periodically
  EVERY_N_SECONDS( 5 ) {
    float reportedFPS = FastLED.getFPS();
    Serial << F("FPS reported (Hz): ") << reportedFPS << endl;
  }
}
