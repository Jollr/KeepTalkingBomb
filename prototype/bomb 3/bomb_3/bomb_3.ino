enum digitalTransition { SAME, LOW_TO_HIGH, HIGH_TO_LOW };
enum ledColor { YELLOW, RED, BLUE };
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
gameState nextState = SEQ1;
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
const ledColor simonSequence[simonStepCount] = { YELLOW, BLUE, RED, BLUE, YELLOW };
const int simonLedMap[3] = {6, 1, 4};
const int simonButtonSequence[simonStepCount] = { 1, 2, 0, 2, 1 };
byte previousLedPin = 0;
const unsigned long simonSequenceLength = simonStepCount * 2 * 300 + 1000;

void seq1LogicStep() {  
  ledWrites[previousLedPin] = LOW;
  int stepInAnimation = stateTimer / 300;
  bool anyButtonIsPressed = buttonReads[0] == HIGH  || buttonReads[1] == HIGH  || buttonReads[2] == HIGH;
  if (stepInAnimation % 2 == 0 && stepInAnimation <= 2*simonStepNumber  && simonButtonCounter < 1 && !anyButtonIsPressed) {
    previousLedPin = simonLedMap[simonSequence[stepInAnimation / 2]];
    ledWrites[previousLedPin] = HIGH;
  } 

  int expectedLed = simonLedMap[simonSequence[simonButtonCounter]];
  if (buttonTransitions[simonButtonSequence[simonButtonCounter]] == HIGH_TO_LOW) {  
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

void seq2() {
  requireNoSwitchTransitions();
  requireNoWireCuts();

  seq2Game();
}

const int seq2CombCount = 8;
int seq2CombCounter = 0;
bool ledCombinations[numLedValues][seq2CombCount] = {
  {false, true, false, true, false, true, true, true},
  {true, false, true, true, false, true, false, true},
  {true, false, true, false, true, true, false, false},
  {false, false, false, true, false, true, true, true},
  {false, false, false, true, true, true, false, true},
  {false, true, false, false, true, true, false, true},
  {false, true, true, false, false, true, false, false},
  {true, false, true, false, false, false, false, true}
};

bool seq2PressedButtons[numButtonPins] = {false, false, false};
const int releasableButtons[seq2CombCount] = { -1, 2, -1, -1, 0, -1, -1, 1 };
void seq2Game() {
  seq2Presentation();
  seq2Logic();
}

void seq2Presentation() {
  for (int n = 0; n < numLedValues; n++) {
    ledWrites[n] = ledCombinations[n][seq2CombCounter];
  }
}

void seq2Logic() {
  for (int n = 0; n < numButtonPins; n++) {
    if (buttonTransitions[n] != HIGH_TO_LOW) continue;
    if (!releasableButtons[seq2CombCounter] == n) {
      log("high to low transition for button ", false);
      log(n, false);
      log(" is not allowed in combination ", false);
      log(seq2CombCounter, true);
      nextState = DEAD;
      return;
    } else {
      seq2PressedButtons[n] = true;
    }
  }

  if (seq2PressedButtons[0] && seq2PressedButtons[1] && seq2PressedButtons[2]) {
    nextState = SEQ3;
    return;
  }

  if (stateTimer > 2000) {
    stateTimer = 0;
    seq2CombCounter++;

    if (seq2CombCounter >= seq2CombCount) {
      seq2CombCounter = 0;
    }
    log("seq 2 combination -> ", false);
    log(seq2CombCounter, true);
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

int expectedSwitchFlipIndex = -1;
int switchFlipOrder[numSwitchPins] = {2, 1, 0};
void seq3Logic() {
  for (int n = 0; n < numSwitchPins; n++) {
    if (switchTransitions[n] == LOW_TO_HIGH) {
      nextState = DEAD;
      return;
    }
  }

  if (expectedSwitchFlipIndex == -1) {
    requireNoSwitchTransitions();
    if (wireCuts[0]) {
      expectedSwitchFlipIndex++;
      log("expectedSwitchFlipIndex ", false);
      log(expectedSwitchFlipIndex, true);
    }
    return;
  }
  
  for (int n = 0; n < numSwitchPins; n++) {
    if (switchTransitions[n] == SAME) {
      continue;
    }

    if (switchTransitions[n] == HIGH_TO_LOW) {
      if (n != switchFlipOrder[expectedSwitchFlipIndex]) {
        log("switch ", false);
        log(n, false);
        log(" high to low not allowed", true);
        nextState = DEAD;
      } else {
        expectedSwitchFlipIndex++;
        log("expectedSwitchFlipIndex ", false);
        log(expectedSwitchFlipIndex, true);
      }
    }
  }

  if (expectedSwitchFlipIndex == 3) {
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
    int newInput = readAnalogPin(n);

    if (abs(analogReads[n] - newInput) > 3) {
      /*log(n, false);
      log(" - ", false);
      log(newInput, true);*/
      analogReads[n] = newInput;
    }
  }
}

int readAnalogPin(int pinNumber) {
  if (pinNumber == 0) return analogRead(pinNumber);
  if (pinNumber == 1) return analogRead(pinNumber);
  if (pinNumber == 2) return 1024 - analogRead(pinNumber);
  if (pinNumber == 3) return 1024 - analogRead(pinNumber);
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
    
    if (ledWrites[ledMap(n)] == HIGH) {
      writeValue++;
    }
  }

  writeTo595(writeValue);
}

int ledMap(int index) {
  
  switch (index) {
    case 0: return 6;
    case 1: return 0;
    case 2: return 1;
    case 3: return 7;
    case 4: return 5;
    case 5: return 3;
    case 6: return 2;
    case 7: return 4;
    default: return 0;
  }
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
