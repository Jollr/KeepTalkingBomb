#include "pitches.h"

typedef enum {OFF, RED, GREEN, BLUE, REDANDGREEN, REDANDBLUE, GREENANDBLUE, ALL} rgb_leg_state;

// pin numbers
const int buttonPin = 2;
const int ledPin = 13;
const int rgbLedPinRed = 10;
const int rgbLedPinGreen = 11;
const int rgbLedPinBlue = 12;
const int irLedPin = 5;
const int irIndicatorLedPin = 6;
const int soundPin = 4;

// program state
int prevButtonState = false;
int buttonPushCounter = 0;
int timeStep = 700;
unsigned long prevTimeStamp = millis();
unsigned long timeCounter = 0;
unsigned long prevIrTimeStamp = millis();
unsigned long irTimeCounter = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(rgbLedPinRed, OUTPUT);
  pinMode(rgbLedPinGreen, OUTPUT);
  pinMode(rgbLedPinBlue, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(irLedPin, INPUT);
  pinMode(irIndicatorLedPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("test");
}

void loop() {
  updateButtonCounter();
  updateButtonLed();

  updateTimeCounter();
  updateRgbLeds();

  sound();

  delay(1);
}

void updateButtonLed() {
  if (buttonPushCounter == 3) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
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

void setRgbLeds(rgb_leg_state state) {
  switch(state) {
    case OFF:
      digitalWrite(rgbLedPinRed, LOW);
      digitalWrite(rgbLedPinGreen, LOW);
      digitalWrite(rgbLedPinBlue, LOW);
      break;
    case RED:
      digitalWrite(rgbLedPinRed, HIGH);
      digitalWrite(rgbLedPinGreen, LOW);
      digitalWrite(rgbLedPinBlue, LOW);
      break; 
    case GREEN:
      digitalWrite(rgbLedPinRed, LOW);
      digitalWrite(rgbLedPinGreen, HIGH);
      digitalWrite(rgbLedPinBlue, LOW);
      break; 
    case BLUE:
      digitalWrite(rgbLedPinRed, LOW);
      digitalWrite(rgbLedPinGreen, LOW);
      digitalWrite(rgbLedPinBlue, HIGH);
      break; 
    case REDANDGREEN:
      digitalWrite(rgbLedPinRed, HIGH);
      digitalWrite(rgbLedPinGreen, HIGH);
      digitalWrite(rgbLedPinBlue, LOW);
      break; 
    case REDANDBLUE:
      digitalWrite(rgbLedPinRed, HIGH);
      digitalWrite(rgbLedPinGreen, LOW);
      digitalWrite(rgbLedPinBlue, HIGH);
      break; 
    case GREENANDBLUE:
      digitalWrite(rgbLedPinRed, LOW);
      digitalWrite(rgbLedPinGreen, HIGH);
      digitalWrite(rgbLedPinBlue, HIGH);
      break; 
    case ALL:
      digitalWrite(rgbLedPinRed, HIGH);
      digitalWrite(rgbLedPinGreen, HIGH);
      digitalWrite(rgbLedPinBlue, HIGH);
      break; 
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
  
  /*int halfPeriod = 13; //one period at 38.5khZ is aproximately 26 microseconds
  int cycles = 38; //26 microseconds * 38 is more or less 1 millisecond
  int i;
  for (i=0; i <=cycles; i++)
  {
    digitalWrite(triggerPin, HIGH); 
    delayMicroseconds(halfPeriod);
    digitalWrite(triggerPin, LOW); 
    delayMicroseconds(halfPeriod - 1);     // - 1 to make up for digitaWrite overhead    
  }
  return digitalRead(readPin);*/
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
  
  //Serial.println(irTimeCounter);
  /*if (timeCounter < timeStep) {
    //tone(soundPin, 440, timeStep);
  }*/

  int length = 100;
  if (irTimeCounter < 1 * timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 2*timeStep) {
    tone(soundPin, NOTE_GS3, length);
  } else if (irTimeCounter < 3*timeStep) {
    tone(soundPin, NOTE_FS3, length);
  } else if (irTimeCounter < 4*timeStep) {
    tone(soundPin, NOTE_B2, length);
  } else if (irTimeCounter < 5.5*timeStep) {
    return;
  }

  else if (irTimeCounter < 6*timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 7*timeStep) {
    tone(soundPin, NOTE_FS3, length);
  } else if (irTimeCounter < 8*timeStep) {
    tone(soundPin, NOTE_GS3, length);
  } else if (irTimeCounter < 9*timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 10.5*timeStep) {
    return;
  }

  else if (irTimeCounter < 11*timeStep) {
    tone(soundPin, NOTE_GS3, length);
  } else if (irTimeCounter < 12*timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 13*timeStep) {
    tone(soundPin, NOTE_FS3, length);
  } else if (irTimeCounter < 14*timeStep) {
    tone(soundPin, NOTE_B2, length);
  } else if (irTimeCounter < 15.5*timeStep) {
    return;
  }

  else if (irTimeCounter < 16*timeStep) {
    tone(soundPin, NOTE_B2, length);
  } else if (irTimeCounter < 17*timeStep) {
    tone(soundPin, NOTE_FS3, length);
  } else if (irTimeCounter < 18*timeStep) {
    tone(soundPin, NOTE_GS3, length);
  } else if (irTimeCounter < 19*timeStep) {
    tone(soundPin, NOTE_E3, length);
  } else if (irTimeCounter < 20.5*timeStep) {
    return;
  }
}

