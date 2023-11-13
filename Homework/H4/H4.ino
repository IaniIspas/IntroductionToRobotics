const int latchPin = 11; 
const int clockPin = 10; 
const int dataPin = 12; 
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;
int displayDigits[] = {segD1, segD2, segD3, segD4};
const int displayCount = 4;
const int encodingsNumber = 16;
byte byteEncodings[encodingsNumber] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
};

const int startStopButton = 1; 
const int resetButton = 2; 
const int lapButton = 3; 

bool running = false;
int lapIndex = 0;
int lapViewIndex = -1;
unsigned long startTime = 0;
unsigned long pausedTime = 0;
float lapTimes[4] = {0, 0, 0, 0};

const unsigned long debounceInterval = 200; 
unsigned long lastStartStopPressTime = 0;
unsigned long lastResetPressTime = 0;
unsigned long lastLapPressTime = 0;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  writeReg(byteEncodings[0]);
  Serial.begin(9600);

  pinMode(startStopButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);
  pinMode(lapButton, INPUT_PULLUP);
}

void loop() {
  unsigned long currentTime = millis();

  if (!digitalRead(startStopButton)) {
    if (currentTime - lastStartStopPressTime > debounceInterval) {
      lastStartStopPressTime = currentTime;
      if (running) {
        pausedTime = currentTime - startTime;
        running = false;
      } else {
        startTime = currentTime - pausedTime;
        running = true;
        lapViewIndex = -1; 
      }
    }
  }

  if (!digitalRead(resetButton) && !running) {
    if (currentTime - lastResetPressTime > debounceInterval) {
      lastResetPressTime = currentTime;
      if (lapViewIndex >= 0) {
        for (int i = 0; i < 4; i++) {
          lapTimes[i] = 0;
        }
        lapIndex = 0;
        lapViewIndex = -1;
      } else {
        pausedTime = 0;
        startTime = currentTime;
      }
    }
  }

  if (!digitalRead(lapButton)) {
    if (currentTime - lastLapPressTime > debounceInterval) {
      lastLapPressTime = currentTime;
      if (running) {
        for (int i = 0; i < 3; i++) {
          lapTimes[i] = lapTimes[i + 1];
        }
        lapTimes[3] = (currentTime - startTime) / 1000.0;
      } else {
        lapViewIndex = (lapViewIndex + 1) % 4;
      }
    }
  }

  if (running) {
    writeNumber((currentTime - startTime) / 100);
  } else if (lapViewIndex >= 0) {
    writeNumber(lapTimes[lapViewIndex] * 10);
  } else {
    writeNumber(pausedTime / 100);
  }
}


void writeReg(int digit) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) {
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(int number) {
  int currentNumber = number;
  int displayDigit = 3; 
  int lastDigit = 0;
  while (displayDigit >= 0) {
    if (currentNumber != 0) {
      lastDigit = currentNumber % 10;
      currentNumber /= 10;
    } else {
      lastDigit = 0; 
    }
    activateDisplay(displayDigit);
    writeReg(byteEncodings[lastDigit]);
    displayDigit--;
    writeReg(B00000000); 
  }
}



