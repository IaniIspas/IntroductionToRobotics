const int redLedPin = 9;
const int greenLedPin = 10;
const int blueLedPin = 11;

const int redPotPin = A0;
const int greenPotPin = A1;
const int bluePotPin = A2;

int redPotValue = 0;
int greenPotValue = 0;
int bluePotValue = 0;


void setup() {
  pinMode(redPotPin, INPUT);
  pinMode(greenPotPin, INPUT);
  pinMode(bluePotPin, INPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
}

void updateChannelValue(int &potValue, int ledPin, int potPin) {
  potValue = analogRead(potPin);
  int newPotValue = map(potValue, 0, 1023, 0, 255); //equivalent with newPotValue = potValue / 4.0
  analogWrite(ledPin, newPotValue);
}

void loop() {
  updateChannelValue(redPotValue, redLedPin, redPotPin);
  updateChannelValue(greenPotValue, greenLedPin, greenPotPin);
  updateChannelValue(bluePotValue, blueLedPin, bluePotPin);
}
