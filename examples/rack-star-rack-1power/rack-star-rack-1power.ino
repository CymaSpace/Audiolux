#include <pixl.h>
#include <FastLED.h>
#include <Logging.h>
#include <Audio.h>

// Uncomment to turn off assertions
#define NDEBUG

// Weird thing I had to do to get the Logging library working with teensy
extern "C" {
  int _getpid() { return -1; }
  int _kill(int pid, int sig) { return -1; }
  int _write() { return -1; }
}

#define LOGLEVEL LOG_LEVEL_INFOS

using namespace pixl;

// Curtains
LEDStrip strip11 = LEDStrip(150);
LEDStrip strip12 = LEDStrip(150);
LEDStrip strip13 = LEDStrip(150);
LEDStrip strip14 = LEDStrip(150);

// Curtain LEDs
LEDs leds11 = LEDs(&strip11, 0, 150);
LEDs leds12 = LEDs(&strip12, 0, 150);
LEDs leds13 = LEDs(&strip13, 0, 150);
LEDs leds14 = LEDs(&strip14, 0, 150);

// Triangles strip, in series, 3 groups of 3 triangles, inner -> outer -> outer
#define TRIANGLE 55
#define GROUP TRIANGLE * 3
#define STRIP GROUP * 3

#define IN_START1 36
#define IN_START2 0
#define IN_START3 18
#define IN_LENGTH1 19
#define IN_LENGTH2 18
#define IN_LENGTH3 18

#define OUT1_START1 37
#define OUT1_START2 0
#define OUT1_START3 19
#define OUT1_LENGTH1 18
#define OUT1_LENGTH2 19
#define OUT1_LENGTH3 18

#define OUT2_START1 19
#define OUT2_START2 37
#define OUT2_START3 0
#define OUT2_LENGTH1 18
#define OUT2_LENGTH2 18
#define OUT2_LENGTH3 19

Input* input;

int circle_length = 30;

LEDStrip strip = LEDStrip(STRIP);
LEDStrip strip10 = LEDStrip(150); // LED Mic
LEDStrip* strips[] = {&strip, &strip, &strip};

// Inner triangles
int in_starts1[] = {
  IN_START1 + (GROUP * 0),
  IN_START2 + (GROUP * 0),
  IN_START3 + (GROUP * 0)
};
int in_starts2[] = {
  IN_START1 + (GROUP * 1),
  IN_START2 + (GROUP * 1),
  IN_START3 + (GROUP * 1)
};
int in_starts3[] = {
  IN_START1 + (GROUP * 2),
  IN_START2 + (GROUP * 2),
  IN_START3 + (GROUP * 2)
};
int in_lengths[] = {IN_LENGTH1, IN_LENGTH2, IN_LENGTH3};

LEDs leds1 = LEDs(3, strips, in_starts1, in_lengths);
LEDs leds2 = LEDs(3, strips, in_starts2, in_lengths);
LEDs leds3 = LEDs(3, strips, in_starts3, in_lengths);

LEDs* inner_leds[] = {&leds1, &leds2, &leds3};

// First outer triangle
int out1_starts1[] = {
  OUT1_START1 + TRIANGLE + (GROUP * 0),
  OUT1_START2 + TRIANGLE + (GROUP * 0),
  OUT1_START3 + TRIANGLE + (GROUP * 0)
};
int out1_starts2[] = {
  OUT1_START1 + TRIANGLE + (GROUP * 1),
  OUT1_START2 + TRIANGLE + (GROUP * 1),
  OUT1_START3 + TRIANGLE + (GROUP * 1)
};
int out1_starts3[] = {
  OUT1_START1 + TRIANGLE + (GROUP * 2),
  OUT1_START2 + TRIANGLE + (GROUP * 2),
  OUT1_START3 + TRIANGLE + (GROUP * 2)
};
int out1_lengths[] = {OUT1_LENGTH1, OUT1_LENGTH2, OUT1_LENGTH3};

LEDs leds4 = LEDs(3, strips, out1_starts1, out1_lengths);
LEDs leds5 = LEDs(3, strips, out1_starts2, out1_lengths);
LEDs leds6 = LEDs(3, strips, out1_starts3, out1_lengths);

// Second outer triangle
int out2_starts1[] = {
  OUT2_START1 + (TRIANGLE * 2) + (GROUP * 0),
  OUT2_START2 + (TRIANGLE * 2) + (GROUP * 0),
  OUT2_START3 + (TRIANGLE * 2) + (GROUP * 0)
};
int out2_starts2[] = {
  OUT2_START1 + (TRIANGLE * 2) + (GROUP * 1),
  OUT2_START2 + (TRIANGLE * 2) + (GROUP * 1),
  OUT2_START3 + (TRIANGLE * 2) + (GROUP * 1)
};
int out2_starts3[] = {
  OUT2_START1 + (TRIANGLE * 2) + (GROUP * 2),
  OUT2_START2 + (TRIANGLE * 2) + (GROUP * 2),
  OUT2_START3 + (TRIANGLE * 2) + (GROUP * 2)
};
int out2_lengths[] = {OUT2_LENGTH1, OUT2_LENGTH2, OUT2_LENGTH3};

LEDs leds7 = LEDs(3, strips, out2_starts1, out2_lengths);
LEDs leds8 = LEDs(3, strips, out2_starts2, out2_lengths);
LEDs leds9 = LEDs(3, strips, out2_starts3, out2_lengths);

LEDs* outer_leds[] = {&leds4, &leds5, &leds6, &leds7, &leds8, &leds9};

// Circle
LEDs circle_led = LEDs(&strip10, 0, circle_length);
LEDs* circle_leds[] = {&circle_led};
CircleAnimation* circle_anim;

Visualization* viz;

CurtainAnimation* anim11; //LED Curtains
CurtainAnimation* anim12;
CurtainAnimation* anim13;
CurtainAnimation* anim14;
TriangleAnimation* anim5; //Cymatic Star
TriangleAnimation* anim6;

// Audio shield setup
AudioInputI2S audio;
AudioAnalyzePeak peak;
AudioAnalyzeNoteFrequency note;
AudioConnection patchCord1(audio, peak);
AudioConnection patchCord2(audio, note);
AudioControlSGTL5000 audioShield;

void setup() {
  Log.Init(LOGLEVEL, 9600);
  delay(1000);
  Log.Info("Starting setup()\n");
  Serial.flush();
  delay(1000);

  AudioMemory(30);
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(63);
  //audioShield.inputSelect(AUDIO_INPUT_LINEIN);
  //audioShield.lineInLevel(15);
  note.begin(.75);

  input = new AudioShieldInput(&peak, &note);

  viz = new RippleVisualization(input, 45, 1, true);

  anim11 = new CurtainAnimation(viz, leds11); // LED Curtains
  anim12 = new CurtainAnimation(viz, leds12);
  anim13 = new CurtainAnimation(viz, leds13);
  anim14 = new CurtainAnimation(viz, leds14);
  anim5 = new TriangleAnimation(viz, inner_leds, 3); // Cymatic Star
  anim6 = new TriangleAnimation(viz, outer_leds, 6);

  // Adds LED Microphone
  circle_anim = new CircleAnimation(viz, circle_leds, 1);

  // Far right curtain
  anim11->init(
     25,    // height in pixels
     6,     // width in pixels
     1.0,   // height in ratio of visualiation
     0.25,  // width in ratio of visualiation
     0.0,   // rotation in radians
     0.75,  // x distance from visualization start in ratio of visualization
    -0.55,   // y "
     0.0);  // z "

  // Inner right curtain
  anim12->init(
     25,    // height in pixels
     6,     // width in pixels
     1.0,   // height in ratio of visualiation
     0.33,  // width in ratio of visualiation
     0.0,   // rotation in radians
    -0.88,  // x distance from visualization start in ratio of visualization
    -0.5,   // y "
     0.0);  // z "

  // Inner Left curtain
  anim13->init(
     25,    // height in pixels
     6,     // width in pixels
     1.0,   // height in ratio of visualiation
     0.33,  // width in ratio of visualiation
     0.0,   // rotation in radians
     0.5,  // x distance from visualization start in ratio of visualization
    -0.5,   // y "
     0.0);  // z "

  // Far left curtain
  anim14->init(
     25,    // height in pixels
     6,     // width in pixels
     1.0,   // height in ratio of visualiation
     0.25,  // width in ratio of visualiation
     0.0,   // rotation in radians
     0.75,  // x distance from visualization start in ratio of visualization
    -0.55,   // y "
     0.0);  // z "

  // Cymatic Star
  anim5->init(1.0);
  anim6->init(0.9, true);

  // LED Mic
  circle_anim->init(1.0);

  //FastLED.addLeds<WS2811, 7, RGB>(strip11.leds, 150); // LED Curtain
  FastLED.addLeds<WS2811, 7, RGB>(strip12.leds, 150);
  FastLED.addLeds<WS2811, 14, RGB>(strip13.leds, 150);
  //FastLED.addLeds<WS2811, 21, RGB>(strip14.leds, 150);
  FastLED.addLeds<WS2811, 2, GRB>(strip.leds, STRIP); // Cymatic Star
  //FastLED.addLeds<WS2811, 6, GRB>(strip10.leds, 150); // LED Mic

  FastLED.setBrightness(100);

  Looper* looper = Looper::instance();
  looper->addInput(input);
  looper->addVisualization(viz);
  looper->addAnimation(anim11); // LED Curtains
  looper->addAnimation(anim12);
  looper->addAnimation(anim13);
  looper->addAnimation(anim14);
  looper->addAnimation(anim5); // Cymatic Star
  looper->addAnimation(anim6);
  looper->addAnimation(circle_anim); // LED Mic
  looper->setFramesPerSecond(45);
  looper->setUpdatesPerSecond(60);

  Log.Info("Finished setup()\n");
  delay(1000);
}

void loop() {
  AudioNoInterrupts();
  Looper::instance()->loop();
  AudioInterrupts();
  delay(5);
}
