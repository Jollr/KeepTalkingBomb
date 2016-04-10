// pin numbers
const int buttonPin = 2;
const int ledPin = 13;

int buttonPushCounter = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  // read the state of the pushbutton value:
  int buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
}

/*void slumber(int ms) {
  bool prevState = false;
  int n = 0;
  for (n = 0; n < ms; n++) {
    delay(1);
    
  }
}
*/
