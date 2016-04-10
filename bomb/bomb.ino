// pin numbers
const int buttonPin = 2;
const int ledPin = 13;

int prevButtonState = false;
int buttonPushCounter = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  updateButtonCounter();
  if (buttonPushCounter == 3) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }

  delay(1);
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

