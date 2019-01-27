// Code by Andrew Brodsky; adapted from "strandtest" sketch included with the Adafruit NeoPixel library
// and "SparkFun_Spectrum_Demo" sketch from Sparkfun's Spectrum Analyzer shield hookup guide.
//
// Note to self: make sure that the power source for the LED strips and the microcontroller providing
// the data line share a common ground! :/

// ––––––––––––––––––––––––––––––––––––––––––––––––––––

// Include the NeoPixel library. Set up a strip.
#include <Adafruit_NeoPixel.h>
#define NUMLEDS 60
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, 7, NEO_GRB + NEO_KHZ800);

// Pin declarations for the spectrum analyzer chips. Note that the strobe and reset pins for each chip
// are tied together.
#define STROBE 4
#define RESET 5
#define LEFTINPUT A0
#define RIGHTINPUT A1 

// Create arrays that will store band values from each channel:
int leftChannel[7]; 
int rightChannel[7];

// Some global variables relevant for strip coloring:
uint32_t stripColor;
int colorVal=0;
int currentCycle=0;

void setup(){
  
  // Start the serial monitor:
  Serial.begin(9600);

  // Set pin modes for the spectrum analyzer chips' inputs and outputs:
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(LEFTINPUT, INPUT);
  pinMode(RIGHTINPUT, INPUT);

  // Initialize the spectrum analyzer chips:
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);

  // Initialize the LED strip:
  strip.begin();
  strip.setBrightness(255);
  strip.show();
}

void loop(){
  getFreqs();
  displayUpdate();
}

void getFreqs(){ // Reads the amplitude of all 7 frequency bands from both channels.
  
  for (int band=0; band<7; band++){
    
    leftChannel[band] = analogRead(LEFTINPUT);
    //Serial.print("left: ");
    //Serial.print(analogRead(LEFTINPUT));
    rightChannel[band] = analogRead(RIGHTINPUT);
    //Serial.print(" right:");
    //Serial.println(analogRead(RIGHTINPUT));
    
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

void displayUpdate(){ // Updates the LED strip to match the most recent amplitude data.

  // The idea is this: The strip will have a continuously changing base color, while the amplitude
  // of bass frequencies determine how many of the LEDs are lighted, starting from the ends of the 
  // strip and going towards the middle.

  // Determine background color. We implement a delay variable to adjust how fast the color changes:
  int delayCycle = 2;
  if(delayCycle==currentCycle){
    currentCycle=0;
    stripColor = rainbowVal(colorVal);
    colorVal++;
    //Serial.println("changing color");
  }
  else{currentCycle++;}

  // Now we initialize the background to the current stripColor:
  for(int i=0; i<NUMLEDS; i++){
    strip.setPixelColor(i, stripColor);
    //Serial.println("resetting strip");
  }

  // Sample signal amplitude:
  int bassAmp=0;
  for (int band=1; band<4; band++){ // Sample the 2nd, 3rd, and 4th channel
    bassAmp += leftChannel[band]+rightChannel[band];
  }
  
  // Map the amplitude data to the number of LEDs that should be lighted on each side:
  bassAmp = map(bassAmp, 0, 6138, 0, floor(NUMLEDS/2));

  // Light the appropriate number of LEDs on each side:
  for(int i=0; i<bassAmp; i++){
    //Serial.println("setting foreground");
    strip.setPixelColor(i, 0, 0, 0);
    strip.setPixelColor(59-i, 0, 0, 0);
  }

  // Finally, update the strip:
  strip.show();

  // Debug:
  Serial.print(" strip color:");
  Serial.print(stripColor);
  Serial.print(" colorVal:");
  Serial.print(colorVal);
  Serial.print(" bassAmp:");
  Serial.println(bassAmp);
}

// This function creates a certain rainbow color from an 8-bit input. Wholeheartedly copied from strandtest.
uint32_t rainbowVal(byte colorVal) {
  colorVal = 255 - colorVal;
  if(colorVal < 85) {
    return strip.Color(255 - colorVal * 3, 0, colorVal * 3);
  }
  if(colorVal < 170) {
    colorVal -= 85;
    return strip.Color(0, colorVal * 3, 255 - colorVal * 3);
  }
  colorVal -= 170;
  return strip.Color(colorVal * 3, 255 - colorVal * 3, 0);
}
