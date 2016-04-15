// pin numbers
const int controlPins[4] = {2, 3, 4, 5};
const int clockPin = 6;
const int latchPin = 7;
const int dataPin = 8;

// program state
unsigned long prevTimeReading;
unsigned long timeCounter = 0;
int output = 0;
int timeDigits[4] = {0, 0, 0, 0};

void setup() {
  setPinModes();
  initializeState();
}

void setPinModes() {
  for (int n = 0; n < 4; n++) {
    pinMode(controlPins[n], OUTPUT);
  }

  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void initializeState() {
  digitalWrite(controlPins[0], HIGH);
  digitalWrite(controlPins[1], LOW);
  digitalWrite(controlPins[2], LOW);
  digitalWrite(controlPins[3], LOW);
  
  prevTimeReading = millis();
}

void loop() {
  update7Segment();
}


void update7Segment() {
  writeSegment(0, 111);
  writeSegment(1, 222);
  writeSegment(2, 180);
  writeSegment(3, 3);
}

void writeSegment(int segment, int value) {
  for (int n = 0; n < 4; n++) {
    digitalWrite(controlPins[n], LOW);
  }
  
  digitalWrite(controlPins[segment], HIGH);
  
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  digitalWrite(latchPin, HIGH);
}
