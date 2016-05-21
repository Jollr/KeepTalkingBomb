//7-segment constants
typedef enum {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, A, E} sevenSegmentValue;

// input pins
const int potentioMeterPin = A0;

// 595 controlling 7-segment
const int latchPin = 8;
const int dataPin = 7;
const int clockPin = 9;

void setup() {
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}

void loop() {
  int val = analogRead(potentioMeterPin);  

  Serial.println(val);
  update7Segment();
  delay(10);
}

void update7Segment() {
  digitalWrite(latchPin, LOW);
  int timeStep = (millis() / 500) % 12;
  int numberToDisplay = get7SegmentOutputValue(timeStep);

  shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
  digitalWrite(latchPin, HIGH);
}

int get7SegmentOutputValue(int logicalValue) {
  switch(logicalValue) {
    case ZERO: return 64+32+16+8+4+2; break;
    case ONE: return 16+2; break;
    case TWO: return 32+16+128+8+4; break;
    case THREE: return 32+16+128+2+4; break;
    case FOUR: return 64+128+16+2; break;
    case FIVE: return 32+64+128+2+4; break;
    case SIX: return 32+64+8+4+2+128; break;
    case SEVEN: return 32+16+2; break;
    case EIGHT: return 128+64+32+16+8+4+2; break;
    case NINE: return 128+64+32+16+2+4; break;
    case A: return 128+64+32+16+2+8; break;
    case E: return 128+64+32+8+4; break;
    default: return 0;
  }
}

