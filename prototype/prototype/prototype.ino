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
  delay(200);
}

void update7Segment() {
  digitalWrite(latchPin, LOW);
  int numberToDisplay = 255;

  shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
  digitalWrite(latchPin, HIGH);
}
