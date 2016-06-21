#include "pitches.h"

enum digitalTransition { SAME, LOW_TO_HIGH, HIGH_TO_LOW };
enum ledColor { YELLOW, RED, BLUE };
bool logging = true;
asdf
// inputs
const int numAnalogPins = 4;
const int analogPins[numAnalogPins] = { A0, A1, A2, A3 };
const int numButtonPins = 3;
const int buttonPins[numButtonPins] = { 2, 3, 4 };
digitalTransition buttonTransitions[numButtonPins] = { SAME, SAME, SAME };
const int numSwitchPins = 3;
const int switchPins[numSwitchPins] = { 5, 6, 7 };
digitalTransition switchTransitions[numSwitchPins] = { SAME, SAME, SAME };
const int numWirePins = 2;
const int wirePins[numWirePins] = { 9, 10 };
int analogReads[numAnalogPins];
int buttonReads[numButtonPins];
int switchReads[numSwitchPins];
bool wireCuts[numWirePins] = {false, false};

// outputs
const int speakerPin = 8;
const int numLedValues = 8;
int ledWrites[numLedValues] = {LOW};

// 595 controlling output leds
const int latchPin = 11;
const int dataPin = 12;
const int clockPin = 13;

// bomb state
enum bomb_level { BOMB1, BOMB2, BOMB3 };
bool switchFlag = false;
bomb_level bombLevel;
enum gameState { DISARMED, SEQ1, SEQ2, SEQ3, DEAD };
gameState currentState = SEQ1;
gameState nextState = SEQ1;
unsigned long stateTimer = 0;
unsigned long prevTimeStamp = millis();

void setup() {
  Serial.begin(9600);
  
  bombLevel = BOMB1;
  currentState = SEQ1;
  
  log("begin", true);
  log("bomb: ", false);
  log(bombLevel, true);
  log("state: ", false);
  log(getStateString(currentState), true);

  for (int n = 0; n < numButtonPins; n++) {
    pinMode(buttonPins[n], INPUT);
  }

  for (int n = 0; n < numSwitchPins; n++) {
    pinMode(switchPins[n], INPUT);
  }

  for (int n = 0; n < numWirePins; n++) {
    pinMode(wirePins[n], INPUT);
  }
  
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);

  logging = false;
  readInputs();
  logging = true;
}

void loop() {
  readInputs();
  gameLogic();
  writeOutputs();
}

void gameLogic() {
  updateStateTimer();

  switch(currentState) {
    case DISARMED: disarmed(); break;
    case DEAD: dead(); break;
    case SEQ1: seq1(); break;
    case SEQ2: seq2(); break;
    case SEQ3: seq3(); break;
  }

  stateTransition();
}

bool bomb1Seq1ButtonFlag = false;
void bomb1Seq1() {
  requireNoWireCuts();
  bomb1Seq1SwitchCheck();
  
  if (switchFlag) {
    bomb1Seq1LogicStep();
  }
}

void bomb1Seq1SwitchCheck() {
  if (!switchFlag || bomb1Seq1ButtonFlag) {
    requireNoButtonPresses();
  }
  
  if (switchTransitions[0] == HIGH_TO_LOW || switchTransitions[1] == HIGH_TO_LOW) {
    nextState = DEAD;
  }
  
  if (!switchFlag && switchReads[0] == HIGH && switchReads[1] == HIGH && switchReads[2] == HIGH) {
    log("setting switch flag to true", true);
    switchFlag = true;
  }

  if (switchTransitions[2] == HIGH_TO_LOW) {
    if (bomb1Seq1ButtonFlag) {
      nextState = SEQ2;
    } else {
      nextState = DEAD;
    }
  }
}

void bomb1Seq1LogicStep() {
  if (buttonReads[0] == HIGH || buttonReads[2] == HIGH) {
    nextState = DEAD;
    return;
  }

  if (bomb1Seq1ButtonFlag) {
    if (analogReads[1] / 256 != 0 || analogReads[2] / 256 != 3) {
      nextState = DEAD;
    }
  } 
  else if (buttonTransitions[1] == HIGH_TO_LOW) {
    if (analogReads[1] / 256 != 0 || analogReads[2] / 256 != 3) {
      nextState = DEAD;
    }
    
    bomb1Seq1ButtonFlag = true;
  }
}

void bomb1Seq2() {
  requireNoWireCuts();
  bomb1Seq2SwitchCheck();
  
  if (!switchFlag) {
    bomb1Seq2LogicStep();  
  }
}

void bomb1Seq2SwitchCheck() {
  if (!switchFlag) {
    requireNoSwitchTransitions();
  } else {
    requireNoButtonPresses();
  }
  
  if (switchTransitions[0] == HIGH_TO_LOW || switchTransitions[2] == LOW_TO_HIGH) {
    nextState = DEAD;
  }
  
  if (switchFlag && switchTransitions[1] == HIGH_TO_LOW) {
    nextState = SEQ3;
  }
}

// simon says
int simonStepNumber = 0;
int simonButtonCounter = 0;
const int simonStepCount = 5;
const ledColor simonSequence[simonStepCount] = { RED, BLUE, RED, YELLOW, BLUE };
const int simonLedMap[3] = {6, 1, 4};
const int simonButtonMap[3] = { 2, 1, 0 };
byte previousLedPin = 0;

void bomb1Seq2LogicStep() {  
  ledWrites[previousLedPin] = LOW;
  int stepInAnimation = stateTimer / 300;
  if (stepInAnimation % 2 == 0 && stepInAnimation <= 2*simonStepNumber) {
    previousLedPin = simonLedMap[simonSequence[stepInAnimation / 2]];
    ledWrites[previousLedPin] = HIGH;
  } 

  int expectedLed = simonLedMap[simonSequence[simonButtonCounter]];
  if ( 
       (buttonTransitions[0] == HIGH_TO_LOW && expectedLed == 1) ||
       (buttonTransitions[1] == HIGH_TO_LOW && expectedLed == 4) ||
       (buttonTransitions[2] == HIGH_TO_LOW && expectedLed == 6)
     ) {
    
    simonButtonCounter++;
    
  } else if (buttonTransitions[0] == HIGH_TO_LOW || buttonTransitions[1] == HIGH_TO_LOW || buttonTransitions[2] == HIGH_TO_LOW) {
    log("wrong button pressed", true);    
    nextState = DEAD;
  }

  if (simonButtonCounter > simonStepNumber) {
    simonButtonCounter = 0;
    simonStepNumber++;
    stateTimer = 0;
  }

  if (simonStepNumber == simonStepCount) {
    simonStepNumber = 0;
    simonButtonCounter = 0;
    previousLedPin = 0;
    switchFlag = true;
    log("setting switch flag to true", true);
  }
}

void bomb1Seq3() {
  requireNoButtonPresses();
  if (wireCuts[0]) nextState = DEAD;
  if (!switchFlag) requireNoSwitchTransitions();

  if (switchTransitions[1] == LOW_TO_HIGH || switchTransitions[2] == LOW_TO_HIGH) {
    nextState = DEAD;
  }

  if (switchTransitions[0] == HIGH_TO_LOW) {
    if (wireCuts[1]) nextState = DISARMED;
    else nextState = DEAD;
  }
}

void bomb2Seq1() {
  bomb2Seq1Logic();
  bomb2Seq1Morse();
}

void bomb2Seq1Logic() {
  if (buttonReads[0] == HIGH || buttonReads[1] == HIGH) {
    nextState = DEAD;
    return;
  }
 
  if (buttonTransitions[2] != HIGH_TO_LOW) return;
  
  if (analogReads[0] / 256 != 1 || 
      analogReads[1] / 256 != 3 ||
      analogReads[2] / 256 != 3 ||
      analogReads[3] / 256 != 2) 
  {
    log(analogReads[0] / 256, true);
    log(analogReads[1] / 256, true);
    log(analogReads[2] / 256, true);
    log(analogReads[3] / 256, true);
    nextState = DEAD;
    return;
  }

  nextState = SEQ2;
}

// morse
const int morseALength = 5;
bool baseMorseA[morseALength] = {true, false, true, true, true};
const int morseCLength = 12;
bool baseMorseC[morseCLength] = {true, true, true, false, true, false, true, true, true, false, true};
const int morseKLength = 9;
bool baseMorseK[morseKLength] = {true, true, true, false, true, false, true, true, true };
const int morseLLength = 10;
bool baseMorseL[morseLLength] = {true, false, true, true, true, false, true, false, true};
const int morseMLength = 8;
bool baseMorseM[morseMLength] = {true, true, true, false, true, true, true};
const int morseOLength = 11;
bool baseMorseO[morseOLength] = {true, true, true, false, true, true, true, false, true, true, true};
const int morseQLength = 17;
bool baseMorseQ[morseQLength] = {true, true, true, false, true, true, true, false, true, false, true, true, true};
const int morseRLength = 8;
bool baseMorseR[morseRLength] = {true, false, true, true, true, false, true};

const int morseUnitLength = 300;
const int morseTotalLength = morseCLength + 3 + morseOLength + 3 + morseLLength + 3 + morseALength + 17;
bool morseButtonFlag = false;

void bomb2Seq1Morse() {
  int morseStep = stateTimer / morseUnitLength;
  
  if (   morseLetter(baseMorseC, morseCLength, 0, morseStep) 
      || morseLetter(baseMorseO, morseOLength, morseCLength + 3, morseStep)
      || morseLetter(baseMorseL, morseLLength, morseCLength + morseOLength + 2*3, morseStep)
      || morseLetter(baseMorseA, morseALength, morseCLength + morseOLength + morseKLength + 3*3, morseStep)      
  ) {
    playSound();
  }
  else {
    stopSound();
  }

  if (morseStep > morseTotalLength) {
    stateTimer = 0;
  }
}

bool morseLetter(bool* letter, int arrayLength, int offset, int morseStep) {
  if (morseStep >= offset && morseStep < offset + arrayLength) {
    return letter[morseStep - offset];
  }

  return false;
}

void bomb2Seq2() {
  bomb2Seq2Leds();
  bomb2Seq2Logic();
}

void bomb2Seq2Logic() {
  if (buttonReads[1] == HIGH || buttonReads[2] == HIGH) {
    nextState = DEAD;
    return;
  }

  if (buttonTransitions[0] != HIGH_TO_LOW) return;
  
  if (switchReads[0] != HIGH || switchReads[1] != HIGH || switchReads[2] != LOW) {
    nextState = DEAD;
    return;
  }

  nextState = SEQ3;
}

void bomb2Seq2Leds() {
  ledWrites[1] = HIGH;
  ledWrites[2] = HIGH;
  ledWrites[5] = HIGH;

  if (stateTimer / 1000 % 2 > 0) {
    ledWrites[6] = HIGH;
    ledWrites[7] = HIGH;
  } else {
    ledWrites[6] = LOW;
    ledWrites[7] = LOW;
  }
}

int bomb2Seq3Step = 1;
void bomb2Seq3() {
  bomb2Seq3Step1();
  bomb2Seq3Step2();
}

void bomb2Seq3Step1() {
  if (bomb2Seq3Step != 1) return;

  if (buttonTransitions[0] == HIGH_TO_LOW || buttonTransitions[1] == HIGH_TO_LOW || buttonTransitions[2] == HIGH_TO_LOW)
  {
    nextState = DEAD; 
    return;
  }

  if (buttonReads[0] == HIGH && buttonReads[1] == HIGH && buttonReads[2] == HIGH) {
    bomb2Seq3Step = 2;
    stateTimer = 0;
    return;
  }
}

const int bomb2Seq3CombCount = 4;
int bomb2Seq3CombCounter = 0;
bool allowedButton1TransitionsPerCombination[numButtonPins][bomb2Seq3CombCount] = {
  {true, false, false, true},
  {false, false, false, true},
  {true, true, false, false}
};

void bomb2Seq3Step2() {
  if (bomb2Seq3Step != 2) return;

  bomb2Seq3Presentation();
  if (buttonTransitions[0] == LOW_TO_HIGH || buttonTransitions[1] == LOW_TO_HIGH || buttonTransitions[2] == LOW_TO_HIGH)
  {
    nextState = DEAD; 
    return;
  }

  for (int n = 0; n < numButtonPins; n++) {
    if (buttonTransitions[n] != HIGH_TO_LOW) continue;
    if (!allowedButton1TransitionsPerCombination[n, bomb2Seq3CombCounter])     {
      nextState = DEAD;
      return;
    }
  }

  if (buttonReads[0] == LOW && buttonReads[1] == LOW && buttonReads[2] == LOW) {
    nextState = DISARMED;
    return;
  }

  if (stateTimer > 5000) {
    stateTimer = 0;
    bomb2Seq3CombCounter++;

    if (bomb2Seq3CombCounter >= bomb2Seq3CombCount) {
      bomb2Seq3CombCounter = 0;
    }
  }
}

bool bomb2Seq3LedDisplays[3][bomb2Seq3CombCount] = {
  {false, true, false},
  {true, true, false},
  {true, true, false}
};

void bomb2Seq3Presentation() {
  for (int n = 0; n < 3; n++) {
    setBomb2Seq3Led(n, bomb2Seq3LedDisplays[n][bomb2Seq3CombCounter]);
  }
}

void setBomb2Seq3Led(int led, bool on) {
  int value = LOW;
  if (on) value = HIGH;
  
  switch(led) {
    case 0: {
      ledWrites[0] = value;
      ledWrites[0] = value;
      break;
    }
    case 1: {
      ledWrites[2] = value;
      ledWrites[3] = value;
      ledWrites[4] = value;
      break;
    }
    case 2: {
      ledWrites[5] = value;
      ledWrites[6] = value;
      ledWrites[7] = value;
      break;
    }
  }
}

void bomb3Seq1() {
  
}

void bomb3Seq2() {
  
}

void bomb3Seq3() {
  
}

void disarmed() {
  int interval = 500;
  
  for (int n = 0; n < numLedValues; n++) {
    ledWrites[n] = LOW;
  }

  ledWrites[(stateTimer % interval*numLedValues) / interval] = HIGH;

  // todo: play sound?
}

void dead() {
  for (int n = 0; n < numLedValues; n++) {
    ledWrites[n] = HIGH;
  }

  // todo: play sound?
}

void stateTransition() {
  if (currentState != nextState) {
    log(getStateString(currentState), false);
    log(" -> ", false);
    log(getStateString(nextState), true);

    switchFlag = false;
    currentState = nextState;
    morseButtonFlag = false;
    
    stateTimer = 0;
    stopSound();
    for (int n = 0; n < numLedValues; n++) {
      ledWrites[n] = LOW;
    }
  }
}

char* getStateString(gameState state) {
  switch (state) {
    case SEQ1: return "seq1";
    case SEQ2: return "seq2";
    case SEQ3: return "seq3";
    case DEAD: return "dead";
    case DISARMED: return "disarmed";
    default: log("unknown state", false); log(state, true); return "";
  }
}

void updateStateTimer() {
  unsigned long timeStamp = millis();
  stateTimer = stateTimer + (timeStamp - prevTimeStamp);
  prevTimeStamp = timeStamp;
}

void seq1() {
  switch (bombLevel) {
    case BOMB1: bomb1Seq1(); return;
    case BOMB2: bomb2Seq1(); return;
    case BOMB3: bomb3Seq1(); return;
  }
}

void seq2() {
  switch (bombLevel) {
    case BOMB1: bomb1Seq2(); return;
    case BOMB2: bomb2Seq2(); return;
    case BOMB3: bomb3Seq2(); return;
  }
}

void seq3() {
  switch (bombLevel) {
    case BOMB1: bomb1Seq3(); return;
    case BOMB2: bomb2Seq3(); return;
    case BOMB3: bomb3Seq3(); return;
  }
}

void requireNoButtonPresses() {
  for (int n = 0; n < numButtonPins; n++) {
    if (buttonTransitions[n] == LOW_TO_HIGH) {
      nextState = DEAD;
    }
  }
}

void requireNoSwitchTransitions() {
  for (int n = 0; n < numSwitchPins; n++) {
    if (switchTransitions[n] != SAME) nextState = DEAD;
  }
}

void requireNoWireCuts() {
  for (int n = 0; n < numWirePins; n++) {
    if (wireCuts[n]) nextState = DEAD;
  }
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
      log(newInput, true);
      analogReads[n] = newInput;
    }
  }
}

unsigned long buttonTransitionTimer[numButtonPins] = {0};
void readDigitalInputs() { 
  for (int n = 0; n < numButtonPins; n++) {
    if (stateTimer - buttonTransitionTimer[n] < 200) {
      buttonTransitions[n] = SAME;
      continue;
    }

    int newRead = digitalRead(buttonPins[n]);
    
    if (newRead != buttonReads[n]) {
      buttonTransitionTimer[n] = stateTimer;
      log(stateTimer, false);
      log(" - ", false);
      
      if (newRead == HIGH) {
        buttonTransitions[n] = LOW_TO_HIGH;
        log("button ", false);
        log(n, false);
        log(" low to high", true);
      }
      else {
        buttonTransitions[n] = HIGH_TO_LOW;
        log("button ", false);
        log(n, false);
        log(" high to low", true);
      }
    }
    else {
      buttonTransitions[n] = SAME;
    }
    
    buttonReads[n] = newRead;
  }

  for (int n = 0; n < numSwitchPins; n++) {
    int newRead = digitalRead(switchPins[n]);
   
    if (newRead != switchReads[n]) {
      log(stateTimer, false);
      log(" - ", false);
      
      if (newRead == HIGH) {
        switchTransitions[n] = LOW_TO_HIGH;
        
        log("switch ", false);
        log(n, false);
        log(" low to high", true);
      }
      else {
        switchTransitions[n] = HIGH_TO_LOW;
        log("switch ", false);
        log(n, false);
        log(" high to low", true);
      }
    }
    else {
      switchTransitions[n] = SAME;
    }
    
    switchReads[n] = newRead;
  }
}

unsigned long wireTransitionTimer[numButtonPins] = {0};
void readWireCuts() {
  for (int n = 0; n < numWirePins; n++) {
    if (stateTimer - wireTransitionTimer[n] < 300) {
      continue;
    }
    
    bool newRead = digitalRead(wirePins[n]) == LOW;
    
    if (newRead != wireCuts[n]) {
      wireTransitionTimer[n] = stateTimer;
      log(stateTimer, false);
      log(" - ", false);
      
      if (newRead) {
        log("wire cut ", false);
        log(n, false);
        log(" false to true", true);
      } else {
        log("wire cut ", false);
        log(n, false);
        log(" true to false", true);
      }
    }
   
    wireCuts[n] = newRead;
  }
}

void writeOutputs() {
  writeLeds();
  controlSound();
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

unsigned long soundStartTime = 0;
int soundPeriod = 0;
void controlSound() {
  if (soundPeriod == 0) return;

  unsigned long now = micros();
  if ( ((now - soundStartTime) / soundPeriod) % 2 == 0) {
    digitalWrite(speakerPin, HIGH);
  } else {
    digitalWrite(speakerPin, LOW);
  }
}

bool playingSound = false;
void playSound() {
  if (playingSound) return;

  playingSound = true;
  soundStartTime = micros();
  soundPeriod = 1000000 / NOTE_G3; // microseconds
}

void stopSound() {
  playingSound = false;
  soundPeriod = 0;
  digitalWrite(speakerPin, LOW);
}

void log(char* message, bool endLine) {
  if (logging) {
    Serial.print(message);
    if (endLine) Serial.println("");
  }
}

void log(int message, bool endLine) {
  if (logging) {
    Serial.print(message);
    if (endLine) Serial.println("");
  }
}
