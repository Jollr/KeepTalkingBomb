enum digitalTransition { SAME, LOW_TO_HIGH, HIGH_TO_LOW };
enum ledColor { YELLOW, BLUE, RED  };
bool logging = true;

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
bool switchFlag = false;
enum gameState { DISARMED, SEQ1, SEQ2, SEQ3, DEAD };
gameState currentState = SEQ1;
gameState nextState = SEQ3;
unsigned long stateTimer = 0;
unsigned long prevTimeStamp = millis();

void setup() {
  Serial.begin(9600);
  
  currentState = SEQ1;
  
  log("begin", true);
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

void seq1() {
  requireNoWireCuts();
  requireNoSwitchTransitions();

  seq1LogicStep();  
}

// simon says
int simonStepNumber = 0;
int simonButtonCounter = 0;
const int simonStepCount = 5;
const ledColor simonSequence[simonStepCount] = { BLUE, YELLOW, RED, YELLOW, BLUE };
const int simonLedMap[3] = {6, 4, 1};
const int simonButtonMap[3] = { 0, 2, 1 };
byte previousLedPin = 0;
const unsigned long simonSequenceLength = simonStepCount * 2 * 300 + 1000;

void seq1LogicStep() {  
  ledWrites[previousLedPin] = LOW;
  int stepInAnimation = stateTimer / 300;
  bool anyButtonIsPressed = buttonReads[0] == HIGH  || buttonReads[1] == HIGH  || buttonReads[2] == HIGH;
  if (stepInAnimation % 2 == 0 && stepInAnimation <= 2*simonStepNumber && simonButtonCounter < 1 && !anyButtonIsPressed) {
    previousLedPin = simonLedMap[simonSequence[stepInAnimation / 2]];
    ledWrites[previousLedPin] = HIGH;
  } 

  int expectedLed = simonLedMap[simonSequence[simonButtonCounter]];
  if ( 
       (buttonTransitions[simonButtonMap[0]] == HIGH_TO_LOW && expectedLed == 1) ||
       (buttonTransitions[simonButtonMap[1]] == HIGH_TO_LOW && expectedLed == 4) ||
       (buttonTransitions[simonButtonMap[2]] == HIGH_TO_LOW && expectedLed == 6)
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
    nextState = SEQ2;
  }

  if (stateTimer > simonSequenceLength) stateTimer = 0;
}

int seq2Step = 1;
const int seq2RequiredButton = 2;
void seq2() {
  requireNoWireCuts();
  requireNoSwitchTransitions();

  for (int n = 0; n < numButtonPins; n++) {
    if (n != seq2RequiredButton && buttonReads[n] == HIGH) {
      nextState = DEAD;
      return;
    }
  }

  if (seq2Step == 1) {
    seq2Step1();
  }
  else if (seq2Step == 2) {
    seq2Step2();
  }
}

void seq2Step1() {
  if (buttonTransitions[seq2RequiredButton] == LOW_TO_HIGH) {
    seq2Step = 2;
    stateTimer = 0;
    log("entering step 2", true);
  }
}

const int seq2AnalogPin = 3;
const int numSeq2Steps = 8;
const int seq2StepLength = 1200;
const int seq2LedsSequence[numSeq2Steps][numLedValues] = {
  {HIGH, LOW, HIGH, HIGH, HIGH, LOW, LOW, HIGH },
  {LOW, HIGH, LOW, LOW, HIGH, HIGH, HIGH, HIGH },
  {LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, HIGH },
  {HIGH, LOW, HIGH, HIGH, LOW, HIGH, HIGH, HIGH },
  {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, HIGH },
  {LOW, HIGH, LOW, LOW, HIGH, LOW, LOW, HIGH },
  {LOW, HIGH, LOW, HIGH, LOW, HIGH, HIGH, LOW },
  {HIGH, HIGH, HIGH, LOW, HIGH, LOW, HIGH, HIGH },
};
const int seq2allowedInput[numSeq2Steps] = {2, 2, 1, 0, 2, 3, 1, 3};

void seq2Step2() {
  seq2Step2Logic();
  seq2Step2Presentation();
}

void seq2Step2Logic() {
  if (buttonTransitions[seq2RequiredButton] != HIGH_TO_LOW) return;

  int allowedInputIndex = stateTimer / seq2StepLength;
  log(allowedInputIndex, true);
  if (analogReads[seq2AnalogPin] / 256 != seq2allowedInput[allowedInputIndex])
  {
    nextState = DEAD;
    return;
  }

  nextState = SEQ3;
}

void seq2Step2Presentation() {
  if (stateTimer > numSeq2Steps * seq2StepLength) {
    stateTimer = 0;
  }

  int ledCombinationIndex = stateTimer / seq2StepLength;
  for (int n = 0; n < numLedValues; n++) {
    ledWrites[n] = seq2LedsSequence[ledCombinationIndex][n];
  }
}

int seq3Step = 1;
void seq3() {
  requireNoButtonPresses();
  if (wireCuts[1]) {
    nextState = DEAD; return;
  }

  if (seq3Step == 1) {
    seq3SwitchStep();
  }
  else {
    seq3MorseStep();
  }
}

void seq3SwitchStep() {
  requireNoWireCuts();

  for (int n = 0; n < numSwitchPins; n++) {
    if (switchTransitions[n] == HIGH_TO_LOW) {
      nextState = DEAD;
      return;
    }
  }

  if (switchReads[0] == HIGH && switchReads[1] == HIGH && switchReads[2] == HIGH) {
    seq3Step++;
    log("entering seq 3 morse step", true);
  }
}

void seq3MorseStep() {
  seq3Logic();
  seq3Morse();
}

int expectedSwitchFlip = -1;
void seq3Logic() {
  for (int n = 0; n < numSwitchPins; n++) {
    if (switchTransitions[n] == LOW_TO_HIGH) {
      nextState = DEAD;
      return;
    }
  }

  if (expectedSwitchFlip == -1) {
    requireNoSwitchTransitions();
    if (wireCuts[0]) {
      expectedSwitchFlip++;
      log("expectedSwitchFlip ", false);
      log(expectedSwitchFlip, true);
    }
    return;
  }
  
  for (int n = 0; n < numSwitchPins; n++) {
    if (n != expectedSwitchFlip) {
      if (switchTransitions[n] == HIGH_TO_LOW){
        log("switch ", false);
        log(n, false);
        log(" high to low not allowed", true);
        nextState = DEAD;
      }
      continue;
    }
    
    if (switchTransitions[expectedSwitchFlip] == HIGH_TO_LOW) {
      expectedSwitchFlip++;
      log("expectedSwitchFlip ", false);
      log(expectedSwitchFlip, true);
    }
  }

  if (expectedSwitchFlip == 3) {
    nextState = DISARMED;
  }
}

// morse
const int morseALength = 5;
bool baseMorseA[morseALength] = {true, false, true, true, true};
const int morseELength = 1;
bool baseMorseE[morseELength] = {true};
const int morseKLength = 9;
bool baseMorseK[morseKLength] = {true, true, true, false, true, false, true, true, true };
const int morseLLength = 9;
bool baseMorseL[morseLLength] = {true, false, true, true, true, false, true, false, true};
const int morsePLength = 11;
bool baseMorseP[morsePLength] = {true, false, true, true, true, false, true, true, true, false, true};

const int morseUnitLength = 300;
const int morseTotalLength = morseKLength + 3 + morseALength + 3 + morsePLength + 3 + morseELength + 3 + morseLLength + 17;
bool morseButtonFlag = false;

void seq3Morse() {
  int morseStep = stateTimer / morseUnitLength;
  
  if (   morseLetter(baseMorseK, morseKLength, 0, morseStep) 
      || morseLetter(baseMorseA, morseALength, morseKLength + 3, morseStep)
      || morseLetter(baseMorseP, morsePLength, morseKLength + morseALength + 2*3, morseStep)
      || morseLetter(baseMorseE, morseELength, morseKLength + morseALength + morsePLength + 3*3, morseStep)
      || morseLetter(baseMorseL, morseLLength, morseKLength + morseALength + morsePLength + morseELength + 4*3, morseStep)
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
  soundPeriod = 1000000 / 196; // microseconds
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
