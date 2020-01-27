#include <LiquidCrystal.h>
#include <EEPROM.h>

const int D4 = 5;
const int D5 = 4;
const int D6 = 3;
const int D7 = 2;
const int RS = 12;
const int enable = 11;
//const int V0 = 10;

LiquidCrystal lcd(RS, enable, D4, D5, D6, D7);

const int pinX = A0;
const int pinY = A1;
const int pinSW = 7;
int xValue = 0;
int yValue = 0;
int switchValue = 1;

bool pressedButton = false;
bool yJoyMoved = false;
bool xJoyMoved = false;
int minTreshold = 400;
int maxTreshold = 600;

int level = 0;
int score = 0;
int lives = 3;
int highscore = 0;
int startLevel = 0;

unsigned long currentTime;
unsigned long previousTime = 0;

String playerName = " ";
char incomingByte = 0;

int currentItem = 0;
int check = 0;
bool gameOver = false;
int levelDelay = 5000;
int number;

void displayMenu(LiquidCrystal lcd, int currentItem) {
  lcd.clear();

  lcd.print(currentItem == 0 ? ">" : "");
  lcd.print("Play ");

  lcd.print(currentItem == 1 ? " >" : " ");
  lcd.print("Highscore");

  lcd.setCursor(0, 1);

  lcd.print(currentItem == 2 ? "   >" : "    ");
  lcd.print("Settings    ");
}

void moveMenu() {
  yValue = analogRead(pinY);
  
  if(yValue <= minTreshold && yJoyMoved == false) {
    if(currentItem < 2) {
      currentItem++;
    }
    else {
      currentItem = 0;
    }
    yJoyMoved = true;
  } 
  if(yValue >= maxTreshold && yJoyMoved == false) {
    if(currentItem > 0) {
      currentItem--;
    }
    else {
      currentItem = 2;
    }
    yJoyMoved = true;
  }
  
  if(yValue > minTreshold && yValue < maxTreshold) {
    yJoyMoved = false;
  }
  
  delay(1);
}

void levelMove() {
  xValue = analogRead(pinX);
  
  if(xValue <= minTreshold && xJoyMoved == false) {
    if(startLevel > 0) {
      startLevel--;
    }
    else {
      startLevel = 5;
    }
    xJoyMoved = true;
  }
  if(xValue >= maxTreshold && xJoyMoved == false) {
    if(startLevel < 5) {
      startLevel++;
    }
    else {
      startLevel = 0;
    }
    xJoyMoved = true;
  }
  
  if(xValue > minTreshold && xValue < maxTreshold){
      xJoyMoved = false;
  }
     
  delay(1);
}
void displayStartGame(LiquidCrystal lcd) {
  lcd.clear();
  
  if(currentTime > levelDelay && previousTime == 0) {
    int long quotient = currentTime / levelDelay;
    int long reminder = currentTime % levelDelay;
    previousTime += levelDelay * quotient + reminder;
  }
  else
    if(previousTime == 0) {
     previousTime = currentTime % levelDelay;
    }

  if(!gameOver) {
    lcd.print("Lives:");
    lcd.print(lives);
    lcd.print(" Level:");
    lcd.print(level);
    lcd.setCursor(0, 1);
    lcd.print("    Score:");
    score = level * 3;
    lcd.print(score);
    lcd.print("    ");
  }
  else {
    lcd.print("   Game  Over   ");
    lcd.setCursor(0, 1);
    lcd.print("Congrats");
    lcd.print(playerName);
    lcd.print("!");
    if(score > highscore) {
      highscore = score;
      EEPROM.write(0, highscore);
    }
  }
  if((!gameOver && pressedButton) && currentTime - previousTime > levelDelay) { 
    level++;
    previousTime = currentTime;
    number++;
  }
  if((lives > 0 && gameOver == true) && number == 2) {
   number = 0;
   score = 0;
   lives--;
   gameOver = false;
   displayMenu(lcd, currentItem);
   pressedButton = !pressedButton;
   }
  if(number == 2) {
    gameOver = true;
  }
  delay(50);    
}

void displayHighscore(LiquidCrystal lcd) {
  lcd.clear();
  lcd.print("Highscore:");
  lcd.print(highscore);
  delay(50);
}

// Set the starting level value
void displaySettings(LiquidCrystal lcd) {
  lcd.clear();
  lcd.print("Starting level:");
  if(!gameOver) 
    levelMove();
  lcd.print(startLevel);
  level = startLevel;
  lcd.setCursor(0, 1);
  lcd.print("Name:");
  
// Enter name using serial
  if (Serial.available() > 0) {
    incomingByte = (char)Serial.read();
  if(incomingByte != '\n') {
    playerName += incomingByte;
    }
  }
  lcd.print(playerName);
  delay(50);
}
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  
//  pinMode(V0, OUTPUT);
//  analogWrite(V0, 120);

  pinMode(pinSW, INPUT_PULLUP);
  
  highscore = EEPROM.read(0);
  EEPROM.write(0, 0);
  Serial.begin(9600);

  lcd.print("      GAME      ");
  lcd.setCursor(0, 1);
  lcd.print("   Main  Menu   ");
  delay(2000);
}

void loop() {
  switchValue = digitalRead(pinSW);
  
  currentTime = millis();
  
  if(switchValue == 0 && check == 0) {
      pressedButton = !pressedButton;
      check = 1;
  }
  if(switchValue == 1) {
    check = 0;
  } 
  if(pressedButton == true) {
    if(currentItem == 0) {
      displayStartGame(lcd);
    }
    else {
      if(currentItem == 1) {
          displayHighscore(lcd);
      }
      else {
          displaySettings(lcd);
      }
    }
  }
  else { 
    moveMenu();
    displayMenu(lcd,currentItem);
    delay(50);
  }
  delay(1);
}
