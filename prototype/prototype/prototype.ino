#include "pitches.h"

//7-segment constants

// input pins
const int numAnalogPins = 4;
const int analogPins[numAnalogPins] = { A0, A1, A2, A3 };
const int numButtonPins = 4;
const int buttonPins[numButtonPins] = { 2, 3, 4 };
const int numSwitchPins = 3;
const int switchPins[numSwitchPins] = { 5, 6, 7 };
const int numWirePins = 2;
const int wirePins[numWirePins] = { 9, 10 };
int analogReads[numAnalogPins];
int buttonReads[numButtonPins];
int switchReads[numSwitchPins];
bool wireCuts[numWirePins] = {false, false};

// output pins
const int speakerPin = 8;
const int numLedValues = 8;
int ledWrites[numLedValues] = {LOW};

// 595 controlling 7-segment
const int latchPin = 11;
const int dataPin = 12;
const int clockPin = 13;

// bomb state
// todo

void setup() {
  Serial.begin(9600);

  for (int n = 0; n < numButtonPins; n++) {
    pinMode(buttonPins[n], OUTPUT);
  }

  for (int n = 0; n < numSwitchPins; n++) {
    pinMode(switchPins[n], OUTPUT);
  }
  
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
}

void loop() {
  readInputs();
  gameLogic();
  writeOutputs();
}

void gameLogic() {
  //int analogReads[numAnalogPins];
  //int buttonReads[numButtonPins];
  //int switchReads[numSwitchPins];
  //int ledWrites[numLedValues] = {LOW};

  if (wireCuts[0]) {
    ledWrites[0] = HIGH;
  } else {
    ledWrites[0] = LOW;
  }

  if (wireCuts[1]) {
    ledWrites[1] = HIGH;
  } else {
    ledWrites[1] = LOW;
  }

  /*ledWrites[0] = buttonReads[0];
  ledWrites[1] = buttonReads[1];
  ledWrites[2] = buttonReads[2];
  ledWrites[3] = switchReads[0];
  ledWrites[4] = switchReads[1];
  ledWrites[5] = switchReads[2];*/
}

void readInputs() {
  readDigitalInputs();
  readAnalogInputs();
  readWireCuts();
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

void readDigitalInputs() {
  for (int n = 0; n < numButtonPins; n++) {
    buttonReads[n] = digitalRead(buttonPins[n]);
  }

  for (int n = 0; n < numSwitchPins; n++) {
    switchReads[n] = digitalRead(switchPins[n]);
  }
}

void readWireCuts() {
  for (int n = 0; n < numWirePins; n++) {
    wireCuts[n] = digitalRead(wirePins[n]) == LOW;
  }
}

void writeOutputs() {
  writeLeds();
}

void writeLeds() {
  int writeValue = 0;
  for (int n = 0; n < numLedValues; n++) {
    writeValue = writeValue * 2;
    
    if (ledWrites[n] == HIGH) {
      writeValue++;
    }
  }

  writeTo595(writeValue);
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


