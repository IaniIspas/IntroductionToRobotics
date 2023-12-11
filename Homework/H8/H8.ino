#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);


byte LCDBrightness = 150;
const int LCDBrightnessAddress = 50;
byte matrixBrightness = 2;
const int matrixBrightnessAddress = 0;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;

const int minThreshold = 200;
const int maxThreshold = 600;

const byte moveInterval = 100;
unsigned long lastMoved = 0;

const byte matrixSize = 8;

const byte playerBlinkInterval = 2000;
unsigned long lastPlayerBlinked = 0;
bool playerOn = true;

byte bombXPos = 0;
byte bombYPos = 0;
unsigned long bombPlantedTime = 0;
const byte bombBlinkInterval = 100;
unsigned long lastBombBlinked = 0;
bool bombPlanted = false;

byte specialBombXPos = 7;
byte specialBombYPos = 7;
const byte specialBombBlinkInterval = 100;
unsigned long lastSpecialBombBlinked = 0;
bool specialBombActive = true;

byte matrix[matrixSize][matrixSize] = {};

bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;
bool bombPlacementInitiated = false;
bool bombDefusalInitiated = false;

bool gameOver = false;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int centerJoystickX;
int centerJoystickY;
int joystickDeadZone = 200;

enum JoystickDirection { UP,
                         DOWN,
                         LEFT,
                         RIGHT,
                         CENTERED };

enum GameState {
  WELCOME,
  MENU,
  START_GAME,
  SETTINGS,
  ADJUST_LCD_BRIGHTNESS,
  ADJUST_MATRIX_BRIGHTNESS,
  ABOUT,
  END_MESSAGE
};

GameState gameState = WELCOME;

int selectedOption = 0;
int generated = 0;

JoystickDirection determineJoystickMovement(int x, int y) {
  bool inDeadZone = abs(x - centerJoystickX) < joystickDeadZone && abs(y - centerJoystickY) < joystickDeadZone;
  if (!inDeadZone) {
    if (x < centerJoystickX - joystickDeadZone) return LEFT;
    if (x > centerJoystickX + joystickDeadZone) return RIGHT;
    if (y < centerJoystickY - joystickDeadZone) return UP;
    if (y > centerJoystickY + joystickDeadZone) return DOWN;
  }
  return CENTERED;
}

unsigned long gameStartTime;

void setup() {
  //Serial.begin(9600);
  //I need this for generate maps that are different
  randomSeed(analogRead(0));

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  pinMode(buttonPin, INPUT_PULLUP);

  initMatrix();

  centerJoystickX = analogRead(xPin);
  centerJoystickY = analogRead(yPin);
}

void loop() {
  handleMenu();
}

void handleMenu() {
  switch (gameState) {
    case WELCOME:
      DisplayIntroMessage();
      break;
    case MENU:
      menuOption();
      break;
    case START_GAME:
      if (!generated) {
        generateMap();
        generated = 1;
      }
      startGame();
      break;
    case SETTINGS:
      adjustSettings();
      break;
    case ADJUST_LCD_BRIGHTNESS:
      adjustLCDBrightness();
      break;
    case ADJUST_MATRIX_BRIGHTNESS:
      adjustMatrixBrightness();
      break;
    case ABOUT:
      displayAbout();
      break;
    case END_MESSAGE:
      displayEndMessage();
    default:
      break;
  }
}

void changeGameState(GameState newGameState) {
  lcd.setCursor(1, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.clear();
  gameState = newGameState;
}

void DisplayIntroMessage() {
  lcd.begin(16, 2);
  lcd.print("Welcome to");

  lcd.setCursor(0, 1);
  lcd.print("Defusing Danger");

  unsigned long startTime = millis();
  unsigned long duration = 5000;

  while (millis() - startTime < duration) {}

  lcd.clear();
  changeGameState(MENU);

  gameStartTime = millis();
}

void adjustSettings() {
  lcd.setCursor(0, 0);
  lcd.print("SETTINGS");

  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  JoystickDirection direction = determineJoystickMovement(xValue, yValue);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if (direction == DOWN) {
    selectedOption = (selectedOption + 1) % 2;  // Adjusted for two options
  } else if (direction == UP) {
    selectedOption = (selectedOption - 1 + 2) % 2;  // Adjusted for two options
  } else if (direction == LEFT || direction == RIGHT) {
    changeGameState(MENU);
  }

  if (selectedOption == 0) {
    lcd.print("LCD Brightness");

    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState) {
      lastDebounceTime = millis();
      lastButtonState = reading;
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;

        if (buttonState == LOW) {
          changeGameState(ADJUST_LCD_BRIGHTNESS);
        }
      }
    }
  } else if (selectedOption == 1) {
    lcd.print("Matrix Brightness");

    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState) {
      lastDebounceTime = millis();
      lastButtonState = reading;
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;

        if (buttonState == LOW) {
          changeGameState(ADJUST_MATRIX_BRIGHTNESS);
        }
      }
    }
  }
}

void displayAbout() {
  const char* aboutTextDown[] = {
    "DefusingDanger",
    "Ispas Jany",
    "IaniIspas"
  };

  const char* aboutTextUp[] = {
    "Game Name",
    "Author",
    "Github"
  };

  lcd.clear();

  for (int i = 0; i < 3; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(aboutTextUp[i]);
    lcd.setCursor(0, 1);
    lcd.print(aboutTextDown[i]);

    delay(2000);
  }

  changeGameState(MENU);
}


void initMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = 1;
    }
  }
  updateMatrix();
}

void adjustMatrixBrightness() {
  lcd.setCursor(0, 0);
  lcd.print("Matrix Brightness");
  lcd.setCursor(0, 1);
  lcd.print(matrixBrightness);
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  JoystickDirection direction = determineJoystickMovement(xValue, yValue);
  if (direction == LEFT) {
    matrixBrightness = max(matrixBrightness - 1, 0);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  } else if (direction == RIGHT) {
    matrixBrightness = min(matrixBrightness + 1, 15);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  }

  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  lc.setIntensity(0, matrixBrightness);

  Serial.println(matrixBrightness);

  if (direction == DOWN) {
    lcd.clear();
    changeGameState(SETTINGS);
  }

  lcd.clear();
}

void adjustLCDBrightness() {
  lcd.setCursor(0, 0);
  lcd.print("LCD Brightness");
  lcd.setCursor(0, 1);
  lcd.print(LCDBrightness);
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  JoystickDirection direction = determineJoystickMovement(xValue, yValue);

  if (direction == LEFT) {
    LCDBrightness = max(LCDBrightness - 1, 150);
    EEPROM.put(LCDBrightnessAddress, LCDBrightness);
  } else if (direction == RIGHT) {
    LCDBrightness = min(LCDBrightness + 1, 255);
    EEPROM.put(LCDBrightnessAddress, LCDBrightness);
  }

  EEPROM.get(LCDBrightnessAddress, LCDBrightness);
  analogWrite(d5, LCDBrightness);


  if (direction == DOWN) {
    changeGameState(SETTINGS);
  }
  lcd.clear();
}

void menuOption() {
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 0);
  lcd.print("MENU");

  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  JoystickDirection direction = determineJoystickMovement(xValue, yValue);

  Serial.println(direction);


  if (direction == DOWN) {
    selectedOption = (selectedOption + 1) % 3;
  } else if (direction == UP) {
    selectedOption = (selectedOption - 1 + 3) % 3;
  }

  // if (direction == DOWN) {
  //   if (selectedOption == 0) {
  //     selectedOption = 2;
  //   } else selectedOption = selectedOption - 1;
  // } else if (direction == UP) {
  //   if (selectedOption == 2) {
  //     selectedOption = 0;
  //   } else selectedOption = selectedOption + 1;
  // }

  lcd.setCursor(0, 1);
  lcd.print("                ");  // Clear the second line of the LCD
  lcd.setCursor(0, 1);

  if (selectedOption == 0) {
    lcd.print("Start Game");
  } else if (selectedOption == 1) {
    lcd.print("Settings");
  } else if (selectedOption == 2) {
    lcd.print("About");
  }

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = reading;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        if (selectedOption == 0) {
          selectedOption = 0;
          lcd.clear();
          changeGameState(START_GAME);
        } else if (selectedOption == 1) {
          lcd.clear();
          selectedOption = 0;
          changeGameState(SETTINGS);
        } else if (selectedOption == 2) {
          lcd.clear();
          selectedOption = 0;
          changeGameState(ABOUT);
        }
      }
    }
  }
  gameStartTime = millis();
}


void displayEndMessage() {
  lcd.print("Congratulations");
  lcd.setCursor(0, 1);
  lcd.print("YOU WIN!");
  unsigned long startTime = millis();
  unsigned long duration = 5000;

  while (millis() - startTime < duration) {}

  changeGameState(MENU);
}



void startGame() {
  if (!gameOver) {
    lcd.print("Time : ");
    lcd.setCursor(7, 0);
    unsigned long currentTime = millis();
    lcd.print((currentTime - gameStartTime) / 1000);
    lcd.setCursor(0, 0);
    if (millis() - lastPlayerBlinked > playerBlinkInterval) {
      playerOn = !playerOn;
      lastPlayerBlinked = millis();
    }

    matrix[xPos][yPos] = playerOn;
    lc.setLed(0, xPos, yPos, playerOn);

    if (millis() - lastMoved > moveInterval) {
      updatePositions();
      lastMoved = millis();
    }

    updateMatrix();

    if (specialBombActive && millis() - lastSpecialBombBlinked > specialBombBlinkInterval) {
      matrix[specialBombXPos][specialBombYPos] = !matrix[specialBombXPos][specialBombYPos];
      lastSpecialBombBlinked = millis();
      updateMatrix();
    }

    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;

        if (buttonState == LOW) {
          buttonPressStartTime = millis();
          buttonPressed = true;
          bombPlacementInitiated = true;
          //Defuse the bomb only if the player is next to it
          if ((xPos == 6 && yPos == 7) || (xPos == 7 && yPos == 6)) {
            bombDefusalInitiated = true;
          }
        } else {
          // Button released
          buttonPressed = false;

          if (!bombDefusalInitiated) {
            // Place bomb only if not in defusal mode and bomb placement was initiated
            placeBomb();
          } else {
            unsigned long buttonPressDuration = millis() - buttonPressStartTime;
            unsigned long timeRemaining = 5000 - (millis() - buttonPressStartTime);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Time remaining: ");
            lcd.print(timeRemaining / 1000);
            // 5 seconds for defusal
            if (buttonPressDuration >= 5000) {
              defuseBomb();
            }
          }

          bombPlacementInitiated = false;
          bombDefusalInitiated = false;
        }
      }
    }

    lastButtonState = reading;

    // Blink bombs
    if (bombPlanted && millis() - lastBombBlinked > bombBlinkInterval) {
      matrix[bombXPos][bombYPos] = !matrix[bombXPos][bombYPos];
      lastBombBlinked = millis();
      updateMatrix();
    }

    // Check if it's time to explode the bomb
    if (bombPlanted && millis() - bombPlantedTime > 3000) {
      explodeBomb();
    }
  } else {
    changeGameState(END_MESSAGE);
  }

  lcd.clear();
}

void generateMap() {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      matrix[i][j] = 0;
    }
  }
  matrix[xPos][yPos] = 1;
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      // Check if the current position is not around the player
      if ((abs(i - xPos) > 1 || abs(j - yPos) > 1) && (i != specialBombXPos || j != specialBombYPos) && random(2)) {
        // Place a wall
        matrix[i][j] = 3;
      }
    }
  }
}

void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col] > 0);
    }
  }
}

void updatePositions() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  xLastPos = xPos;
  yLastPos = yPos;

  // Calculate the new position
  byte newXPos = xPos;
  byte newYPos = yPos;

  JoystickDirection direction = determineJoystickMovement(xValue, yValue);

  Serial.println(direction);

  switch (direction) {
    case UP:
      newYPos = (yPos > 0) ? yPos - 1 : matrixSize - 1;
      break;
    case DOWN:
      newYPos = (yPos + 1) % matrixSize;
      break;
    case LEFT:
      newXPos = (xPos + 1) % matrixSize;
      break;
    case RIGHT:
      newXPos = (xPos > 0) ? xPos - 1 : matrixSize - 1;
      break;
    case CENTERED:
      break;
  }

  if ((xPos == 0 && newXPos == matrixSize - 1) || (xPos == matrixSize - 1 && newXPos == 0) || (yPos == 0 && newYPos == matrixSize - 1) || (yPos == matrixSize - 1 && newYPos == 0)) {
    // Prevent horizontal teleportation
    newXPos = xPos;
    // Prevent vertical teleportation
    newYPos = yPos;
  }

  // Only update the position if the new position is not a wall
  if (matrix[newXPos][newYPos] != 3) {
    xPos = newXPos;
    yPos = newYPos;
  }

  // Update matrix
  if (xPos != xLastPos || yPos != yLastPos) {
    updateMatrix();
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
  }
}

void placeBomb() {
  bombXPos = xPos;
  bombYPos = yPos;
  bombPlantedTime = millis();
  bombPlanted = true;
  matrix[bombXPos][bombYPos] = 2;
  updateMatrix();
}

void explodeBomb() {
  matrix[bombXPos][bombYPos] = 0;
  if (bombXPos > 0) matrix[bombXPos - 1][bombYPos] = 0;
  if (bombXPos < matrixSize - 1) matrix[bombXPos + 1][bombYPos] = 0;
  if (bombYPos > 0) matrix[bombXPos][bombYPos - 1] = 0;
  if (bombYPos < matrixSize - 1) matrix[bombXPos][bombYPos + 1] = 0;
  bombPlanted = false;
  updateMatrix();
}

void defuseBomb() {
  specialBombActive = false;
  matrix[bombXPos][bombYPos] = 0;
  gameOver = true;
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = 1;
    }
  }
  updateMatrix();
}

void printMatrix() {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      Serial.print(matrix[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}
