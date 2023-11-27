#include "LedControl.h"

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

byte matrixBrightness = 2;
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

byte matrix[matrixSize][matrixSize] = {};

bool buttonState = HIGH;             
bool lastButtonState = HIGH;         
unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;    

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
  matrix[xPos][yPos] = 1;

  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      // Check if the current position is not around the player
      if ((abs(i - xPos) > 1 || abs(j - yPos) > 1) && random(2)) {
        // Place a wall
        matrix[i][j] = 3; 
      }
    }
  }

  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {

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

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW && !bombPlanted) {
        placeBomb();
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

  if (xValue < minThreshold) {
    newXPos = (xPos + 1) % matrixSize;
  } else if (xValue > maxThreshold) {
    newXPos = (xPos > 0) ? xPos - 1 : matrixSize - 1;
  }

  if (yValue < minThreshold) {
    newYPos = (yPos > 0) ? yPos - 1 : matrixSize - 1;
  } else if (yValue > maxThreshold) {
    newYPos = (yPos + 1) % matrixSize;
  }

  // Check if the new position is on the opposite side of the matrix horizontally or vertically
  if ((xPos == 0 && newXPos == matrixSize - 1) || (xPos == matrixSize - 1 && newXPos == 0) ||
      (yPos == 0 && newYPos == matrixSize - 1) || (yPos == matrixSize - 1 && newYPos == 0)) {
    // Prevent horizontal teleportation
    newXPos = xPos; 
    // Prevent vertical teleportation
    newYPos = yPos;
  }

  // Check if the new position is diagonal and there is a wall nearby
  if (newXPos != xPos && newYPos != yPos) {
    if (matrix[newXPos][yPos] == 3 && matrix[xPos][newYPos] == 3) {
      // Prevent diagonal movement
      newXPos = xPos;
      newYPos = yPos;
    }
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