#include "pitches.h"

typedef enum {OFF, RED, GREEN, BLUE, REDANDGREEN, REDANDBLUE, GREENANDBLUE, ALL} rgb_leg_state;

// pin numbers
const int buttonPin = 2;
const int button2Pin = 3;
const int ledPin = 13;
const int rgbLedPinRed = 10;
const int rgbLedPinGreen = 11;
const int rgbLedPinBlue = 12;
const int irLedPin = 5;
const int soundPin = 4;
const int latchPin = 8;
const int dataPin = 7;
const int clockPin = 9;

// program state
int prevButtonState = false;
int prevButton2State = false;
int buttonPushCounter = 0;
int timeStep = 700;
unsigned long prevTimeStamp = millis();
unsigned long timeCounter = 0;
unsigned long prevIrTimeStamp = millis();
unsigned long irTimeCounter = 0;
unsigned long prevSegmentTimeStamp = millis();
unsigned long segmentTimeCounter = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(rgbLedPinRed, OUTPUT);
  pinMode(rgbLedPinGreen, OUTPUT);
  pinMode(rgbLedPinBlue, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(irLedPin, INPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("test");
}

void loop() {
  updateButtonCounter();
  updateButtonLed();

  updateTimeCounter();
  updateRgbLeds();

  sound();

  update7Segment();
}

void updateButtonLed() {
  if (buttonPushCounter == 3) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void updateButtonCounter() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState != prevButtonState) {
    prevButtonState = buttonState;
    
    if (buttonState == HIGH) {
      buttonPushCounter++;
      if (buttonPushCounter == 4) {
        buttonPushCounter = 0;
      }
    }
  }
}

void updateTimeCounter() {
  unsigned long currentTimeStamp = millis();
  timeCounter = timeCounter + currentTimeStamp - prevTimeStamp;
  prevTimeStamp = currentTimeStamp;
  if (timeCounter > 8*timeStep) {
    timeCounter = 0;
  }
}

void updateRgbLeds() {
  setRgbLeds(getRgbLedState());
}

rgb_leg_state getRgbLedState() {
  if (timeCounter < timeStep) {
    return OFF;
  } else if (timeCounter < 2*timeStep) {
    return RED;
  } else if (timeCounter < 3*timeStep) {
    return GREEN;
  } else if (timeCounter < 4*timeStep) {
    return BLUE;
  } else if (timeCounter < 5*timeStep) {
    return REDANDGREEN;
  } else if (timeCounter < 6*timeStep) {
    return REDANDBLUE;
  } else if (timeCounter < 7*timeStep) {
    return GREENANDBLUE;
  } 
  return ALL;
}

void setRgbLeds(int r, int g, int b) {
  digitalWrite(rgbLedPinRed, r);
  digitalWrite(rgbLedPinGreen, g);
  digitalWrite(rgbLedPinBlue, b);
}

void setRgbLeds(rgb_leg_state state) {
  switch(state) {
    case OFF: setRgbLeds(LOW, LOW, LOW); break;
    case RED: setRgbLeds(HIGH, LOW, LOW); break;
    case GREEN: setRgbLeds(LOW, HIGH, LOW); break;
    case BLUE: setRgbLeds(LOW, LOW, HIGH); break;
    case REDANDGREEN: setRgbLeds(HIGH, HIGH, LOW); break;
    case REDANDBLUE: setRgbLeds(HIGH, LOW, HIGH); break;
    case GREENANDBLUE: setRgbLeds(LOW, HIGH, HIGH); break;
    case ALL: setRgbLeds(HIGH, HIGH, HIGH); break;
  }
}

bool irRead()
{
  int loopCount = 20000;
  int counter = 0;
  for (int i = 0; i < loopCount; i++) {
    counter = counter + digitalRead(irLedPin);
    delayMicroseconds(1);
  }

  return counter < loopCount;
}

void sound() {
  unsigned long now = millis();
  
  if (irRead()) {
    irTimeCounter = irTimeCounter + now - prevIrTimeStamp;
    prevIrTimeStamp = now;
  } else {
    prevIrTimeStamp = now;
    irTimeCounter = 0;
    return;
  }

  int length = 100;
  if (irTimeCounter < 1 * timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 1.5*timeStep) {
    tone(soundPin, NOTE_G3, length);
  } else if (irTimeCounter < 2.5*timeStep) {
    tone(soundPin, NOTE_FS3, length);
  } else if (irTimeCounter < 3*timeStep) {
    tone(soundPin, NOTE_B2, length);
  } else if (irTimeCounter < 4*timeStep) {
    tone(soundPin, NOTE_B2, length);
  } else if (irTimeCounter < 5*timeStep) {
    return;
  }
}

void update7Segment() {
  int button2State = digitalRead(button2Pin);
  unsigned long now = millis();
  int interval = 200;
  int numCombinations = 256;
  
  if (button2State == HIGH) {
    segmentTimeCounter = segmentTimeCounter + now - prevSegmentTimeStamp;
    prevSegmentTimeStamp = now;
    prevButton2State = button2State;
  } else {
    prevSegmentTimeStamp = now;
    prevButton2State = button2State;
    return;
  }

  if (segmentTimeCounter > interval*numCombinations) {
    segmentTimeCounter = 0;
  }

  digitalWrite(latchPin, LOW);
  int numberToDisplay = segmentTimeCounter / interval;

  shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
  digitalWrite(latchPin, HIGH);
}

