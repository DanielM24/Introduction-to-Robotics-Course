const byte pinA = 12;
const byte pinB = 8;
const byte pinC = 5;
const byte pinD = 3;
const byte pinE = 2;
const byte pinF = 11;
const byte pinG = 6;
const byte pinDP = 4;

const byte pinD1 = 7;
const byte pinD2 = 9;
const byte pinD3 = 10;
const byte pinD4 = 13;

const byte segSize = 7;
const byte noOfDisplays = 4;

const byte segmentPins[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG
};

const byte displayPins[noOfDisplays] = {
  pinD1, pinD2, pinD3, pinD4
};

const byte noOfDigits = 10;

const byte pinJSSW = 1;
const byte pinJSX = A0;
const byte pinJSY = A1;

const int jsThreshold = 400;
const int blinkSpeed = 500;

const byte digits[noOfDigits][segSize] = {
// a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}, // 9
};


int switchValue = 0;
int xValue = 0;
int yValue = 0;

byte digitStates[noOfDisplays] = { 0, 0, 0, 0 };
byte dpState = LOW;

byte activeDigit = 0;
bool lockedIn = false;
bool joystickPressed = false;
bool joystickMoved = false;

void selectDisplay(byte index) {
  for (int i = 0; i < noOfDisplays; i++) {
    digitalWrite(displayPins[i], HIGH);
  }
  digitalWrite(displayPins[index], LOW);
}

void displayDigit(byte digit) {
  for (int i = 0; i < segSize; i++) {
    digitalWrite(segmentPins[i], digits[digit][i]);
  }
}

void incrementValue(byte& value, byte threshold) {
  value = value + 1;
  if(value >= threshold){
    value = value % threshold;
  }
}

void decrementValue(byte& value, byte threshold) {
  value = (value - 1 + threshold) % threshold;
}

void updateJoystick() {
  switchValue = digitalRead(pinJSSW);
  xValue = analogRead(pinJSX);
  yValue = analogRead(pinJSY);

  if (switchValue == LOW) {
    if (!joystickPressed) {
      lockedIn = !lockedIn;
    }
    joystickPressed = true;
  } else {
    joystickPressed = false;
  }

  if (lockedIn) {
    if (xValue < 512 - jsThreshold) {
      if (!joystickMoved) {
        decrementValue(digitStates[activeDigit], noOfDigits);
      }
      joystickMoved = true;
    } else if (xValue > 512 + jsThreshold) {
      if (!joystickMoved) {
        incrementValue(digitStates[activeDigit], noOfDigits);
      }
      joystickMoved = true;
    } else {
      joystickMoved = false;
    }
  } else {
    if (yValue < 512 - jsThreshold) {
      if (!joystickMoved) {
        incrementValue(activeDigit, noOfDisplays);
      }
      joystickMoved = true;
    } else if (yValue > 512 + jsThreshold) {
      if (!joystickMoved) {
        decrementValue(activeDigit, noOfDisplays);
      }
      joystickMoved = true;
    } else {
      joystickMoved = false;
    }
  }
}

void updateDP() {
  if (lockedIn) {
    dpState = HIGH;
  } else {
    dpState = (millis() % blinkSpeed) > (blinkSpeed / 2);
  }
}

void updateDigitsDisplay() {
  for (int i = 0; i < noOfDisplays; i++) {
    selectDisplay(i);
    displayDigit(digitStates[i]);
    digitalWrite(pinDP, i == activeDigit ? dpState : LOW);
    delay(5);
  }
}

void setup() {
  pinMode(pinJSSW, INPUT_PULLUP);
  
  for (int i = 0; i < segSize; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  
  pinMode(pinDP, OUTPUT);
  
  for (int i = 0; i < noOfDisplays; i++) {
    pinMode(displayPins[i], OUTPUT);
  }
  
  Serial.begin(9600);
}

void loop() {
  updateJoystick();
  updateDP();
  updateDigitsDisplay();
}
