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
const byte targetFPS = 50;

// maximum brightness
const byte masterBrightness = 255;

// fade amount each update
//byte fadeEverything = 64;
byte fadeEverything = 16;

// see pixelset.h for documention on these classes
// main container
CRGBArray<NUM_LEDS> leds;
// a container per controller
CRGBSet C0( leds(0 * NUM_LEDS_PER_CONTROLLER, 1 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C1( leds(1 * NUM_LEDS_PER_CONTROLLER, 2 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C2( leds(2 * NUM_LEDS_PER_CONTROLLER, 3 * NUM_LEDS_PER_CONTROLLER - 1) );
CRGBSet C3( leds(3 * NUM_LEDS_PER_CONTROLLER, 4 * NUM_LEDS_PER_CONTROLLER - 1) );
// and reversed version; note: C0(C0, -LEN) fails in my hands.
CRGBSet C0r = C0(C0.size() - 1, 0);
CRGBSet C1r = C1(C1.size() - 1, 0);
CRGBSet C2r = C2(C2.size() - 1, 0);
CRGBSet C3r = C3(C3.size() - 1, 0);
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
CRGBSet CTRACK[NUM_CONTROLLERS] = { C0, C1r, C3, C2r };
CRGBSet CTRACKr[NUM_CONTROLLERS] = { C0r, C1, C3r, C2 };

// define three "little tracks". one each for looping around each leg, and one for looping around the top
CRGBSet CTRACKL[NUM_CONTROLLERS / 2] = { CL[0], CL[1](CL[1].size() - 1, 0) };
CRGBSet CTRACKR[NUM_CONTROLLERS / 2] = { CR[0], CR[1](CR[1].size() - 1, 0) };
CRGBSet CTRACKT[NUM_CONTROLLERS] = { CTOP[0], CTOP[1](CTOP[1].size() - 1, 0), CTOP[3], CTOP[2](CTOP[2].size() - 1, 0) };

class Cset : public CRGBSet {
  public:
    void foo();
};

void setup() {
  Serial.begin(115200);

  LEDS.addLeds<WS2811_PORTA, NUM_CONTROLLERS, RGB>(leds, NUM_LEDS_PER_CONTROLLER).setCorrection(TypicalLEDStrip);
  LEDS.setBrightness(masterBrightness);
  FastLED.setMaxRefreshRate(targetFPS);

  pinMode(LED_BUILTIN, OUTPUT);

  // show the memory footprints of the containers
  Serial << F("Size of: leds=") << sizeof(leds) << endl;
  Serial << F("Size of: CTOP=") << sizeof(CTOP) << endl;
  Serial << F("Size of: CLEG=") << sizeof(CLEG) << endl;

  Serial << F("Setup. complete.") << endl;

  while (0) {

    Serial << F("Red.") << endl;
    leds.fill_solid(CRGB(255, 0, 0));
    LEDS.show();
    LEDS.delay(2000);

    Serial << F("Green.") << endl;
    leds.fill_solid(CRGB(0, 255, 0));
    LEDS.show();
    LEDS.delay(2000);

    Serial << F("Blue.") << endl;
    leds.fill_solid(CRGB(0, 0, 255));
    LEDS.show();
    LEDS.delay(2000);

    leds.fadeToBlackBy(255);
    LEDS.show();
    LEDS.delay(1000);
  }
}

// maybe not execute everything all at once
#define RUN_TOP 0
#define RUN_LEGS 0
#define RUN_EXTENT 0
#define RUN_TRACK 0
#define RUN_LOOPS 0
#define RUN_FIRE 0
#define RUN_LEG_JUGGLE 1
#define RUN_OVERHEAD_SPOTS 1
#define RUN_OVERHEAD_RAINBOW 1
#define DEBUG_CONTROLLERS 0

void loop() {
  // 1. fade everything
  leds.fadeToBlackBy(fadeEverything);

  if ( RUN_TOP ) {
    // move a pixel back and forth across the top bar
    static byte topHue = 0; // current Hue
    static int topHueDelta = 5; // Hue steps
    static fract8 topBlur = 128; // 50% blurring
    static int topLoc = random8(CTOP[0].size()); // position. NOTE: NOT using #define's, as I want to work within a container
    static int dir = 1; // directionality
    if ( topLoc + dir > CTOP[0].size() - 1 || topLoc + dir < 0) dir *= -1; // swap directions if needed.
    topLoc += dir; // advance location
    topHue += topHueDelta; // advance hue
    for ( byte i = 0; i < NUM_CONTROLLERS; i++) {
      CTOP[i][topLoc] += CHSV(topHue, 255, 128); // paint
      CTOP[i].blur1d(topBlur); // blur, just for visual interest
    }
  }

  if ( RUN_LEGS ) {
    // show a rainbow on the legs
    static byte legHue = 0;
    static int legHueDelta = 5;
    legHue++;
    CLEG[0].fill_rainbow(legHue, legHueDelta); // paint
    CLEG[0].fadeToBlackBy(196);
    CLEG[1] = CLEG[0]; // mirror
    CLEG[2].fill_rainbow(legHue + 128, -legHueDelta); // paint, noting we're using the other side of the wheel
    CLEG[2].fadeToBlackBy(196);
    CLEG[3] = CLEG[2]; // mirror
  }

  if ( RUN_OVERHEAD_RAINBOW ) {
    // show a rainbow on the legs
    static byte topHue = 0;
    static int topHueDelta = 255/8;
    byte topBrightness = 32;
    
    topHue++;
    CTOP[0].fill_rainbow(topHue, topHueDelta); // paint
    CTOP[0].fadeToBlackBy(255-topBrightness);
    CTOP[1] = CTOP[0]; // mirror
    CTOP[2].fill_rainbow(topHue + 128, -topHueDelta); // paint, noting we're using the other side of the wheel
    CTOP[2].fadeToBlackBy(255-topBrightness);
    CTOP[3] = CTOP[2]; // mirror
  }


  if ( RUN_EXTENT ) {
    // flag the extent of the containers
    static CHSV extentColor = CHSV(HUE_BLUE, 0, 16); // color choice.  faint white.
    for ( byte i = 0; i < NUM_CONTROLLERS; i++) {
      CTOP[i][0] += extentColor;
      CTOP[i][CTOP[i].size() - 1] += extentColor;
      CLEG[i][0] += extentColor;
      CLEG[i][CLEG[i].size() - 1] += extentColor;
    } // paint
  }

  if ( RUN_TRACK ) {
    // send a bright white dot around the big track
    static CHSV trackColor = CHSV(HUE_BLUE, 0, 255); // color choice.  bright white.
    static word trackPos[2] = {random8(NUM_CONTROLLERS), random8(CTRACK[0].size())}; // current position
    trackPos[1]++; // increment
    if ( trackPos[1] > CTRACK[trackPos[0]].size() - 1 ) {
      trackPos[1] = 0; // wrap around
      trackPos[0]++;
      if ( trackPos[0] > NUM_CONTROLLERS - 1 ) trackPos[0] = 0;
    }
    CTRACK[trackPos[0]][trackPos[1]] += trackColor; // paint
  }

  if ( RUN_LOOPS ) {
    static CHSV lTrackColor = CHSV(HUE_RED, 255, 255); // color choice.  bright red.
    static CHSV rTrackColor = CHSV(HUE_GREEN, 255, 255); // color choice.  bright green.
    static CHSV tTrackColor = CHSV(HUE_BLUE, 255, 255); // color choice.  bright blue.

    static word lTrackPos[2] = {random8(NUM_CONTROLLERS / 2), random8(CTRACKL[0].size())}; // current position
    static word rTrackPos[2] = {random8(NUM_CONTROLLERS / 2), random8(CTRACKR[0].size())}; // current position
    static word tTrackPos[2] = {random8(NUM_CONTROLLERS * 2), random8(CTRACKT[0].size())}; // current position

    lTrackPos[1]++; // increment
    if ( lTrackPos[1] > CTRACKL[lTrackPos[0]].size() - 1 ) {
      lTrackPos[1] = 0; // wrap around
      lTrackPos[0]++;
      if ( lTrackPos[0] > NUM_CONTROLLERS / 2 - 1 ) lTrackPos[0] = 0;
    }
    rTrackPos[1]++; // increment
    if ( rTrackPos[1] > CTRACKR[rTrackPos[0]].size() - 1 ) {
      rTrackPos[1] = 0; // wrap around
      rTrackPos[0]++;
      if ( rTrackPos[0] > NUM_CONTROLLERS / 2 - 1 ) rTrackPos[0] = 0;
    }
    tTrackPos[1]++; // increment
    if ( tTrackPos[1] > CTRACKT[tTrackPos[0]].size() - 1 ) {
      tTrackPos[1] = 0; // wrap around
      tTrackPos[0]++;
      if ( tTrackPos[0] > NUM_CONTROLLERS - 1 ) tTrackPos[0] = 0;
    }

    CTRACKL[lTrackPos[0]][lTrackPos[1]] += lTrackColor; // paint
    CTRACKR[rTrackPos[0]][rTrackPos[1]] += rTrackColor; // paint
    CTRACKT[tTrackPos[0]][tTrackPos[1]] += tTrackColor; // paint
  }

  if ( DEBUG_CONTROLLERS ) {
    static int controller = 0;
    switch ( controller ) {
      case 0: C0r[0] += CHSV(HUE_BLUE, 0, 64); break;
      case 1: C1r[0] += CHSV(HUE_BLUE, 0, 64); break;
      case 2: C2r[0] += CHSV(HUE_BLUE, 0, 64); break;
      case 3: C3r[0] += CHSV(HUE_BLUE, 0, 64); break;
    }

    EVERY_N_SECONDS( 1 ) {
      controller += 1;
      if ( controller > NUM_CONTROLLERS - 1 ) controller = 0;
      Serial << F("Controller: ") << controller << endl;
    }
  }

  if( RUN_FIRE ) {
    byte cooling = 196;
    byte sparks = 20;
    Fire2012WithPalette( cooling, sparks );
  }

  if( RUN_LEG_JUGGLE ) {
    byte dots = 5;
    JuggleLegs( dots );
  }

  if( RUN_OVERHEAD_SPOTS ) {
    byte overheadBrightness = 255;
    CHSV overheadColor = CHSV(HUE_BLUE, 0, overheadBrightness);
    
    for ( byte i = 0; i < NUM_CONTROLLERS; i++) {
      CTOP[i][0] = overheadColor;
//      CTOP[i][CTOP[i].size() - 1] = overheadColor;
    } // paint
  }
  
  // blink with fps rate
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);

  // show, delaying as-needed based on target FPS
  LEDS.show();

  // show our frame rate, periodically
  EVERY_N_SECONDS( 5 ) {
    float reportedFPS = FastLED.getFPS();
    Serial << F("FPS reported (Hz): ") << reportedFPS << endl;
  }
}


void JuggleLegs(byte dots) {
  static CRGBSet Fire = CLEG[0](CLEG[0].size() - 1, 0);

  static CRGBPalette16 gPal = HeatColors_p;

  // eight colored dots, weaving in and out of sync with each other
  byte dothue = 0;
  for ( int i = 0; i < dots; i++) {
    //    Fire[beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= CHSV(dothue, 200, 255);
        CLEG[0][beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= ColorFromPalette( gPal, scale8(dothue, 240) );
        dothue += 32;
        CLEG[1][beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= ColorFromPalette( gPal, scale8(dothue, 240) );
        dothue += 32;
        CLEG[2][beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= ColorFromPalette( gPal, scale8(dothue, 240) );
        dothue += 32;
        CLEG[3][beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= ColorFromPalette( gPal, scale8(dothue, 240) );
      dothue += 32;
    // Fire[beatsin16(i + dots - 1, 0, NUM_LEDS_LEG)] += CHSV(HUE_RED, 255, 255);
    //    CTRACK[beatsin16(i+dots-1,0,NUM_LEDS_LEG)] |= ColorFromPalette( gPal, scale8(dothue, 240) );
   // dothue += 32;
  }
  // mirror
//  CLEG[1] = CLEG[0];
//  CLEG[2] = CLEG[0];
//  CLEG[3] = CLEG[0];
}


// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
// #define COOLING 196

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
// #define SPARKING 25

void Fire2012WithPalette(byte cooling, byte sparks) {
  static CRGBPalette16 gPal = HeatColors_p;
//  static CRGBPalette16 gPal = PartyColors_p;

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( analogRead(0) );

  //  static uint8_t hue = 0;
  //  hue++;
  //  CRGB darkcolor  = CHSV(hue, 255, 192); // pure hue, three-quarters brightness
  //  CRGB lightcolor = CHSV(hue, 128, 255); // half 'whitened', full brightness
  //  gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS_LEG];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS_LEG; i++) {
    heat[i] = qsub8( heat[i],  random8(0, (((int)cooling * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS_LEG - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < sparks ) {
    int y = random8(2);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  static CRGBSet Fire = CLEG[0](CLEG[0].size() - 1, 0);
  for ( int j = 0; j < NUM_LEDS_LEG; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    //    byte colorindex = scale8( heat[j]+2, 240);

    //    CLEG[0][j] = ColorFromPalette( gPal, colorindex);
    Fire[j] = ColorFromPalette( gPal, colorindex);
    //      CLEG[0][j] = HeatColor( heat[j] );
  }

  // mirror
  CLEG[1] = CLEG[0];
  CLEG[2] = CLEG[0];
  CLEG[3] = CLEG[0];
}

