enum digitalTransition { SAME, LOW_TO_HIGH, HIGH_TO_LOW };
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
  requireNoSwitchTransitions();
  seq1Logic();
  seq1Morse();
}

void seq1Logic() {
  if (buttonReads[0] == HIGH || buttonReads[2] == HIGH) {
    nextState = DEAD;
    return;
  }
 
  if (buttonTransitions[1] != HIGH_TO_LOW) return;
  
  if (analogReads[0] / 256 != 2 || 
      analogReads[1] / 256 != 3 ||
      analogReads[2] / 256 != 3 ||
      analogReads[3] / 256 != 1) 
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

void seq1Morse() {
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

void seq2() {
  seq2Logic();
}

void seq2Logic() {
  if (buttonReads[1] == HIGH || buttonReads[2] == HIGH) {
    nextState = DEAD;
    return;
  }

  if (buttonTransitions[0] != HIGH_TO_LOW) return;

  log(" 0 - ", false);
  log(switchReads[0], true);
  log(" 1 - ", false);
  log(switchReads[1], true);
  log(" 2 - ", false);
  log(switchReads[2], true);
  
  if (switchReads[0] != LOW || switchReads[1] != HIGH || switchReads[2] != HIGH) {
    nextState = DEAD;
    return;
  }

  nextState = SEQ3;
}

int seq3Step = 1;
void seq3() {
  requireNoSwitchTransitions();
  
  if (buttonTransitions[0] == HIGH || buttonTransitions[1] == HIGH)
  {
    nextState = DEAD; 
    return;
  }
  
  if (seq3Step == 1) {
    seq3Step1();  
  }
  else if (seq3Step == 2) {
    seq3Step2();  
  }
}

void seq3Step1() {
  if (buttonTransitions[2] == LOW_TO_HIGH) {
    log("seq3Step -> 2", true);
    seq3Step = 2;
    stateTimer = 0;
    return;
  }
}

const int seq3CombCount = 8;
int seq3CombCounter = 0;

void seq3Step2() {
  seq3Presentation();
  seq3Step2Logic();
}

void seq3Step2Logic() {
  if (stateTimer > 2000) {
    log("seq 3 combination - ", false);
    log(seq3CombCounter, true);
    stateTimer = 0;
    seq3CombCounter++;

    if (seq3CombCounter >= seq3CombCount) {
      seq3CombCounter = 0;
    }
  }
  
  if (buttonTransitions[2] != HIGH_TO_LOW) return;
  log("button released", true);
    
  if (seq3CombCounter != 2) {
    log("wrong button release; combination counter: ", false);
    log(seq3CombCounter, true);
    nextState = DEAD;
    return;
  }

  nextState = DISARMED;
  
}

bool seq3LedDisplays[numLedValues][seq3CombCount] = {
  {false, true, true, true, false, true, true, true},
  {false, true, false, true, true, true, true, false},
  {true, true, true, true, false, true, false, false},
  {false, false, true, true, false, true, false, true},
  {true, true, false, true, false, true, false, false},
  {false, true, false, true, false, false, false, true},
  {true, false, true, true, true, true, false, false}
};

void seq3Presentation() {
  for (int n = 0; n < numLedValues; n++) {
    int ledValue = seq3LedDisplays[n][seq3CombCounter];
    ledWrites[n] = ledValue;
  }
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
      log(n, false);
      log(" - ", false);
      log(newInput, true);
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
