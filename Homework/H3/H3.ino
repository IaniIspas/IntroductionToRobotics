int displayPins[] = {12, 10, 9, 8, 7, 6, 5, 4};
int controlPins[] = {A0, A1, 2};
 
int joystickDeadZone = 400;
unsigned long ledBlinkInterval = 500;
unsigned long buttonDebounceTime = 50;
int RESET_HOLD_TIME = 3000;
 
int displayState = 7;
bool ledStates[8] = {false};
bool displayBlinkOn = false;
unsigned long lastUpdateMillis = 0;
int centerJoystickX, centerJoystickY;

bool joystickMoved = false;
 
enum JoystickDirection {UP, DOWN, LEFT, RIGHT, CENTERED};
 
const int displayTransition[8][4] = {
  {-1, 6, 5, 1}, 
  {0, 6, 5, -1}, 
  {6, 3, 4, 7}, 
  {6, -1, 4, 2},
  {6, 3, -1, 2}, 
  {0, 6, -1, 1}, 
  {0, 3, -1, -1}, 
  {-1, -1, 2, -1}
};
 
bool isButtonActive = false;
unsigned long buttonActiveTime = 0;
 
void setup() {
  //Serial.begin(9600);
  for (auto pin : displayPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);  
  }
  pinMode(controlPins[2], INPUT_PULLUP);  
 
  centerJoystickX = analogRead(controlPins[0]);
  centerJoystickY = analogRead(controlPins[1]);
}
 
void loop() {
  bool currentButtonState = digitalRead(controlPins[2]);
 
  static bool lastButtonState = HIGH;
  static unsigned long lastButtonTime = millis();
 
  if (currentButtonState != lastButtonState) {
    lastButtonTime = millis();
  }
 
  if ((millis() - lastButtonTime) > buttonDebounceTime) {
    if (currentButtonState == LOW) {
      isButtonActive = true;
      buttonActiveTime = millis();
    } else if (isButtonActive) {
      unsigned long holdTime = millis() - buttonActiveTime;
      if (holdTime >= RESET_HOLD_TIME) {
        for (int i = 0; i < 8; i++) {
          ledStates[i] = false;
        }
        displayState = 7; 
      } else {
        ledStates[displayState] = !ledStates[displayState];
      }
      isButtonActive = false;
    }
  }
 
  lastButtonState = currentButtonState;
 
  int joystickX = analogRead(controlPins[0]);
  int joystickY = analogRead(controlPins[1]);
  JoystickDirection movementDirection = determineJoystickMovement(joystickX, joystickY);

    if (!joystickMoved && movementDirection != CENTERED) {
      int nextState = displayTransition[displayState][movementDirection];
      if (nextState != -1 && nextState != displayState) {
        displayState = nextState;
        joystickMoved = true;  
      }
  }

  if (movementDirection == CENTERED) {
    joystickMoved = false;
  }
 
  updateDisplay();
}
 
void updateDisplay() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateMillis >= ledBlinkInterval) {
    lastUpdateMillis = currentMillis;
    displayBlinkOn = !displayBlinkOn;
  }
 
  for (size_t i = 0; i < 8; i++) {
    if (i == displayState) {
      digitalWrite(displayPins[i], displayBlinkOn ? HIGH : LOW);
    } else {
      digitalWrite(displayPins[i], ledStates[i] ? HIGH : LOW);
    }
  }
}
 
JoystickDirection determineJoystickMovement(int x, int y) {
  bool inDeadZone = abs(x - centerJoystickX) < joystickDeadZone &&
                    abs(y - centerJoystickY) < joystickDeadZone;
  if (!inDeadZone) {
    if (x < centerJoystickX - joystickDeadZone) return LEFT;
    if (x > centerJoystickX + joystickDeadZone) return RIGHT;
    if (y < centerJoystickY - joystickDeadZone) return UP;
    if (y > centerJoystickY + joystickDeadZone) return DOWN;
  }
  return CENTERED;
}