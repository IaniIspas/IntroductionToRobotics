const int floorIndicatorPins[] = {2, 3, 4};
const int operationalStatusLEDPin = 5;
const int floorButtonPins[] = {6, 7, 8};
const int debounceDuration = 50;
const int movementDuration = 3000;
const int elevatorBlinkInterval = 1000;
int currentFloor = 1;
bool isElevatorMoving = false;
unsigned long lastBlinkTime = 0;
unsigned long lastMovementTime = 0;
byte floorButtonStates[3] = {HIGH, HIGH, HIGH};
byte lastFloorButtonStates[3] = {HIGH, HIGH, HIGH};
unsigned long lastDebounceTimes[3] = {0, 0, 0};

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(floorIndicatorPins[i], OUTPUT);
    pinMode(floorButtonPins[i], INPUT_PULLUP);
    digitalWrite(floorIndicatorPins[i], LOW); 
  }
  pinMode(operationalStatusLEDPin, OUTPUT);
  digitalWrite(operationalStatusLEDPin, HIGH);  
  Serial.begin(9600);
  updateFloorLEDs();
}

void loop() {
  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(floorButtonPins[i]);
    if (reading != lastFloorButtonStates[i]) {
      lastDebounceTimes[i] = millis();
    }
    if ((millis() - lastDebounceTimes[i]) > debounceDuration) {
      if (reading != floorButtonStates[i]) {
        floorButtonStates[i] = reading;
        if (floorButtonStates[i] == LOW && !isElevatorMoving) {
          int targetFloor = i + 1;
          if (targetFloor != currentFloor) {
            isElevatorMoving = true;
            moveElevatorToFloor(targetFloor);
          }
        }
      }
    }
    lastFloorButtonStates[i] = reading;
  }
}

void moveElevatorToFloor(int targetFloor) {
  int direction = 1;
  if(currentFloor > targetFloor)
    direction = -1;
  while (currentFloor != targetFloor) {
    if (millis() - lastBlinkTime > elevatorBlinkInterval) {
      digitalWrite(operationalStatusLEDPin, !digitalRead(operationalStatusLEDPin));
      lastBlinkTime = millis();
    }
    if (millis() - lastMovementTime > movementDuration) {
      currentFloor = currentFloor + direction;
      updateFloorLEDs();
      lastMovementTime = millis();
    }
  }
  digitalWrite(operationalStatusLEDPin, HIGH);
  isElevatorMoving = false;
}

void updateFloorLEDs() {
  for (int i = 0; i < 3; i++) {
    if (i == currentFloor - 1) {
      digitalWrite(floorIndicatorPins[i], HIGH);
    } else {
      digitalWrite(floorIndicatorPins[i], LOW);
    }
  }
}









