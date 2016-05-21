// pin numbers
const int controlPins[4] = {5, 2, 3, 4};
const int clockPin = 6;
const int latchPin = 7;
const int dataPin = 8;

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
  digitalWrite(controlPins[0], LOW);
  digitalWrite(controlPins[1], LOW);
  digitalWrite(controlPins[2], LOW);
  digitalWrite(controlPins[3], LOW);
}

void loop() {
  for (int n = 0; n < 10000; n++) {
    for (int m = 0; m < 15; m++) {
      update7Segment(10000-n);
    }
  }
}


void update7Segment(int n) {
  writeSegments((n / 1000) % 10, (n / 100) % 10, (n / 10) % 10, n % 10);
}

void writeSegments(int n1, int n2, int n3, int n4) {
  writeSegment(0, get7SegmentDisplayValue(n1));
  writeSegment(1, get7SegmentDisplayValue(n2));
  writeSegment(2, get7SegmentDisplayValue(n3));
  writeSegment(3, get7SegmentDisplayValue(n4));
}

void writeSegment(int segment, int value) {
  for (int n = 0; n < 4; n++) {
    digitalWrite(controlPins[n], LOW);
  }

  digitalWrite(latchPin, LOW);
  digitalWrite(controlPins[segment], HIGH);
  
  
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  digitalWrite(latchPin, HIGH);
  digitalWrite(controlPins[segment], LOW);
  delay(1);
  
}

int get7SegmentDisplayValue(int num) {
  switch(num) {
    case 0: return 255-(1+4+8+2+32+128); break;
    case 1: return 255-2-32; break;
    case 2: return 255-8-2-16-1-128; break;
    case 3: return 255-8-2-16-32-128; break;
    case 4: return 255-4-16-2-32; break;
    case 5: return 255-8-4-16-32-128; break;
    case 6: return 255-8-4-1-128-32-16; break;
    case 7: return 255-8-2-32; break;
    case 8: return 255-8-4-2-16-32-128-1; break;
    case 9: return 255-4-8-2-16-32-128; break;
    default: return 255-64; break;
  }
}

