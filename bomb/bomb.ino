typedef enum {OFF, RED, GREEN, BLUE, REDANDGREEN, REDANDBLUE, GREENANDBLUE, ALL} rgb_leg_state;

// pin numbers
const int buttonPin = 2;
const int ledPin = 13;
const int rgbLedPinRed = 10;
const int rgbLedPinGreen = 11;
const int rgbLedPinBlue = 12;

// program state
int prevButtonState = false;
int buttonPushCounter = 0;
int timeStep = 700;
unsigned long prevTimeStamp = millis();
unsigned long timeCounter = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(rgbLedPinRed, OUTPUT);
  pinMode(rgbLedPinGreen, OUTPUT);
  pinMode(rgbLedPinBlue, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  updateButtonCounter();
  updateButtonLed();

  updateTimeCounter();
  updateRgbLeds();
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

