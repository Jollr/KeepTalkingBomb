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
 
}

void seq2() {
  
}

void seq3() {
 
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
