#include "pitches.h"

//7-segment constants
typedef enum {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, A, E} sevenSegmentValue;
typedef enum {ALL_OFF, RED, BLUE, YELLOW, RED_AND_BLUE, RED_AND_YELLOW, BLUE_AND_YELLOW, ALL_ON} ledMode;

// input pins
const int potentioMeterPin = A0;

// output pins
const int speakerPin = 6;
const int redLedPin = 3;
const int blueLedPin = 2;
const int yellowLedPin = 4;

// 595 controlling 7-segment
const int latchPin = 8;
const int dataPin = 7;
const int clockPin = 9;

void setup() {
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
}

void loop() {
  int val = analogRead(potentioMeterPin);  

  Serial.println(val);

  
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);

  updateLeds();
  update7Segment();
  //sound();
  delay(20);
}

void updateLeds() {
  writeToLeds(ALL_ON);
  
}

void writeToLeds(ledMode mode) {
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, LOW);
  
  //typedef enum {ALL_OFF, RED, BLUE, YELLOW, RED_AND_BLUE, RED_AND_YELLOW, BLUE_AND_YELLOW, ALL_ON} ledMode;
  switch(mode) {
    case RED: digitalWrite(redLedPin, HIGH); break;
    case BLUE: digitalWrite(blueLedPin, HIGH); break;
    case YELLOW: digitalWrite(yellowLedPin, HIGH); break;
    case RED_AND_BLUE: digitalWrite(redLedPin, HIGH); digitalWrite(blueLedPin, HIGH); break;
    case RED_AND_YELLOW: digitalWrite(redLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
    case BLUE_AND_YELLOW: digitalWrite(blueLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
    case ALL_ON: digitalWrite(redLedPin, HIGH); digitalWrite(blueLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
  }
}

void update7Segment() {
  digitalWrite(latchPin, LOW);
  int timeStep = (millis() / 500) % 12;
  int numberToDisplay = get7SegmentOutputValue(timeStep);

  shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
  digitalWrite(latchPin, HIGH);
}

int get7SegmentOutputValue(int logicalValue) {
  switch(logicalValue) {
    case ZERO: return 64+32+16+8+4+2; break;
    case ONE: return 16+2; break;
    case TWO: return 32+16+128+8+4; break;
    case THREE: return 32+16+128+2+4; break;
    case FOUR: return 64+128+16+2; break;
    case FIVE: return 32+64+128+2+4; break;
    case SIX: return 32+64+8+4+2+128; break;
    case SEVEN: return 32+16+2; break;
    case EIGHT: return 128+64+32+16+8+4+2; break;
    case NINE: return 128+64+32+16+2+4; break;
    case A: return 128+64+32+16+2+8; break;
    case E: return 128+64+32+8+4; break;
    default: return 0;
  }
}

 void sound() {
  if ( (millis() / 500) % 2 == 0 ) {
    tone(speakerPin, NOTE_C2, 20);
  }
 }

