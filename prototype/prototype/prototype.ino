#include "pitches.h"

//7-segment constants
//typedef enum {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, A, E} sevenSegmentValue;
//typedef enum {ALL_OFF, RED, BLUE, YELLOW, RED_AND_BLUE, RED_AND_YELLOW, BLUE_AND_YELLOW, ALL_ON} ledMode;

// input pins
const int numAnalogPins = 4;
const int analogPins[numAnalogPins] = { A0, A1, A2, A3 };

// output pins
const int speakerPin = 4;

// 595 controlling 7-segment
const int latchPin = 11;
const int dataPin = 12;
const int clockPin = 13;

// bomb state
int analogReads[numAnalogPins];

void setup() {
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
}

void loop() {
  //Serial.println(val);

  readAnalogInputs();

  for (int n = 0; n < 256; n++) {
    writeTo595(n);
    delay(50);
  }
}

void readAnalogInputs() {
  for (int n = 0; n < numAnalogPins; n++) {
    int newInput = analogRead(analogPins[n]);

    if (abs(analogReads[n] - newInput) > 3) {
      Serial.println(newInput);
      analogReads[n] = newInput;
    }
  }
}

void writeTo595(int value) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  digitalWrite(latchPin, HIGH);
}

 void sound() {
  if ( (millis() / 500) % 2 == 0 ) {
    tone(speakerPin, NOTE_C2, 20);
  }
 }

/* void writeToLeds(ledMode mode) {
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, LOW);
  
  switch(mode) {
    case RED: digitalWrite(redLedPin, HIGH); break;
    case BLUE: digitalWrite(blueLedPin, HIGH); break;
    case YELLOW: digitalWrite(yellowLedPin, HIGH); break;
    case RED_AND_BLUE: digitalWrite(redLedPin, HIGH); digitalWrite(blueLedPin, HIGH); break;
    case RED_AND_YELLOW: digitalWrite(redLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
    case BLUE_AND_YELLOW: digitalWrite(blueLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
    case ALL_ON: digitalWrite(redLedPin, HIGH); digitalWrite(blueLedPin, HIGH); digitalWrite(yellowLedPin, HIGH); break;
  }
}*/

/*void update7Segment() {
  digitalWrite(latchPin, LOW);
  //int timeStep = (millis() / 500) % 12;
  int val = 10 * analogRead(potentioMeterPin) / 1024;
  int numberToDisplay = get7SegmentOutputValue(val);

  shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
  digitalWrite(latchPin, HIGH);
}*/

/*int get7SegmentOutputValue(int logicalValue) {
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
}*/


