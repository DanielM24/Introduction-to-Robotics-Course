// ---------- VARIABLES AND LIBRARIES ----------
/* The selected pins are based on Arduino ATmega2560*/
#include <SoftwareSerial.h>
// ##### TFT LCD WITH TOUCHSCREEN ######

#include <UTFT.h>
#include <URTouch.h>
#include <UTFT_Buttons.h>
#include "colors.h"

#define LCD_RS 38
#define LCD_WR 39
#define LCD_CS 40
#define LCD_RST 41

#define LCD_MOSI 44
#define LCD_MISO 45
#define LCD_TCLK 46
#define LCD_TCS 47
#define LCD_PEN 49

// Create an instance of the TFT screen library
UTFT    screen(ILI9486,38,39,40,41);
// Create an instance of the touchscreen library
URTouch touch(46, 47, 44, 45, 49);
// Create an instance of the buttons library
UTFT_Buttons button (&screen, &touch);

unsigned int x_cord, y_cord;

// Declare the fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SevenSeg_XXXL_Num[];

// ##### RTC DS 1307 #####

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static int currentHour, currentMinutes, currentSeconds, currentDay, currentMonth, currentYear;

// Create an instance of the RTC library
tmElements_t tm;

// ##### DFPLAYER WITH AMP AND 2 SPEAKERS #####

#include <DFPlayerMini_Fast.h>
const int DFPlayer_RX = 10;
const int DFPlayer_TX = 11;
int iV = 15;
int alarmTrack = 1;
// Create an instance of the DFPlayer library
DFPlayerMini_Fast MP3_Player;

// ##### ALARM MODES #####
int difficulty = 0; // 0 - Eazy, 1 - Hard
/* There are 3 alarm modes: 1 - Math question; 2 - LED memory game; 3 - Pushbutton game;
   Default mode is the Math question
*/
int alarm_mode = 1;
// List of iterators: 
int iterator, iterator_2; 

bool game_done = false;

// ##### ALARM MODE 1 - MATH QUESTION #####
/* The player will receive a math question with 3 possible answers.
   Only one is good, the other two are given to create confusion.
 */
int var1, var2; 
int good_result;
int false_result1, false_result2;
char symbol[] = {'+', '-', '*', '/'};
String problemString;

// ##### ALARM MODE 2 - MEMORY GAME #####
int LEDs[] = {50/*Red*/, 51/*Green*/, 53/*Blue*/, 52/*Yellow*/};
int Buttons[] = {A1 /*Red*/, A2/*Green*/, A3/*Blue*/, A4/*Yellow*/};
int Games_Number = 0;
int Games_Iterator = 0;
int buttonstate = 0;
int last_buttonstate = 0;
int NumberOfLEDs = 4;
int random_array[10]; // Array where we save the random sequence of LEDs
int input_array[10];  // Array where we save the the player input sequence

// ##### ALARM MODE 3 - PUSHBUTTON GAME ##### 
const int Button_Game = A0;
int Button_counter = 0;

// ##### MENU #####
char currentPage, playStatus;

// ##### ALARM #####
int alarm_hour = 0;
int alarm_minutes = 0;
bool alarm_set = false;
String alarmString = "";

// ##### BUTTONS #####
int pressed_button, alarm_button, settings_button, media_button;
int play_button, pause_button, next_button, previous_button, volumeUp_button, volumeDown_button;

// ---------- SETUP ZONE ----------
void setup(){
  Serial.begin(9600);
  
  screen.InitLCD(); // Initialize the display in Landscape mode
  screen.clrScr();
  screen.setBrightness(14);

  touch.InitTouch();
  touch.setPrecision(PREC_MEDIUM);

  button.setTextFont(SmallFont);
  button.setSymbolFont(Dingbats1_XL);
  
  SoftwareSerial mySerial(DFPlayer_RX, DFPlayer_TX);
  mySerial.begin(9600);
  MP3_Player.begin(mySerial);
  setUpMP3();
  
  for (iterator = 0; iterator < NumberOfLEDs; iterator++){
    pinMode(LEDs[iterator],OUTPUT);
  }
  for (iterator = 0; iterator < NumberOfLEDs; iterator++){
    pinMode(Buttons[iterator],INPUT_PULLUP);
  }
  randomSeed(analogRead(A5));
  pinMode(Button_Game, INPUT_PULLUP);

  setUpRTC();
  drawHomeScreen(); // Home Screen function
  currentPage = '0'; // Start with the Home Screeen
  playStatus = '0';  // Nothing to play at the beginning
  currentHour = 0;
  currentMinutes = 0;
  currentDay = 0;
  currentMonth = 0;
  currentYear = 0;
}

// ---------- FUNCTIONS ZONE ----------

// ##### RTC FUNCTIONS #####
void setUpRTC(){ 
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    // and configure the RTC with this info
    RTC.write(tm);
  }
  if (RTC.read(tm)){
    Serial.print("DS1307 Working!");
  }
  else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
  }
}

bool getTime(const char *str) {
  int Hour, Min, Sec;
  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str) {
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;
  
  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

// ##### MP3 FUNCTIONS #####
void setUpMP3(){
  MP3_Player.volume(iV);
  delay(20);
  MP3_Player.sleep();
}

// ##### ALARM MODE 1 - MATH QUESTION #####

void math_game(){
  int symbol_number, pressed_button;
  if (difficulty == 0){
    var1 = random(5, 11);
    var2 = random(8, 16);
    symbol_number = random(0, 4);
    switch(symbol_number){
      case 0:
        good_result = var1 + var2;
        false_result1 = good_result + 1;
        false_result2 = good_result - 2;
        problemString = (String)var1 + " + " + (String)var2 + " = ? ";
        break;
      case 1:
        good_result = var1 - var2;
        false_result1 = good_result + 2;
        false_result2 = good_result - 1;
        problemString = (String)var1 + " - " + (String)var2 + " = ? ";
        break;
      case 2:
        good_result = var1 * var2;
        false_result1 = good_result + var1;
        false_result2 = good_result - var2;
        problemString = (String)var1 + " * " + (String)var2 + " = ? ";
        break;
      case 3:
        good_result = var1 / var2;
        false_result1 = good_result + 1;
        false_result2 = good_result - 1;
        problemString = (String)var1 + " / " + (String)var2 + " = ? ";
        break;
    }
  }

  else{
    var1 = random(5, 15);
    var2 = random(10, 20);
    symbol_number = random(0, 4);
    switch(symbol_number){
      case 0:
        good_result = var1 + var2;
        false_result1 = good_result + 1;
        false_result2 = good_result - 1;
        problemString = (String)var1 + " + " + (String)var2 + " = ? ";
        break;
      case 1:
        good_result = var1 - var2;
        false_result1 = good_result + 1;
        false_result2 = good_result - 1;
        problemString = (String)var1 + " - " + (String)var2 + " = ? ";
        break;
      case 2:
        good_result = var1 * var2;
        false_result1 = good_result + var1;
        false_result2 = good_result - var2;
        problemString = (String)var1 + " * " + (String)var2 + " = ? ";
        break;
      case 3:
        good_result = var1 / var2;
        false_result1 = good_result + 1;
        false_result2 = good_result - 1;
        problemString = (String)var1 + " / " + (String)var2 + " = ? ";
        break;
    }
  }
  screen.clrScr();
  screen.setBackColor(Black); // Sets the background color of the area where the text will be printed to black
  screen.setColor(DarkGreen); 
  screen.setFont(BigFont);
  screen.print(problemString, CENTER, 25);
  screen.printNumI(false_result1, 60, 120);
  screen.printNumI(good_result, 210, 120);
  screen.printNumI(false_result2, 360, 120);
  
  int var_1, var_2, var_3;
  var_1 = button.addButton(40, 50, 140, 100, "VAR 1");
  var_2 = button.addButton(190, 50, 290, 100, "VAR 2");
  var_3 = button.addButton(340, 50, 440, 100, "VAR 3");
  button.setButtonColors(White, White, DarkGreen, White, DarkGreen);
  button.drawButtons();

  while(game_done == false)
  {
    if (touch.dataAvailable() == true){
      pressed_button = button.checkButtons();
      if (pressed_button == var_2)
         game_done = true;
    }
  } 
}

// ##### ALARM MODE 2 - MEMORY GAME #####

void check_fail(){ 
  for (iterator = 0; iterator <= 2; iterator++){
    digitalWrite(LEDs[0], HIGH);
    digitalWrite(LEDs[1], HIGH);
    digitalWrite(LEDs[2], HIGH);
    digitalWrite(LEDs[3], HIGH);
    delay(300);
    digitalWrite(LEDs[0], LOW);
    digitalWrite(LEDs[1], LOW);
    digitalWrite(LEDs[2], LOW);
    digitalWrite(LEDs[3], LOW);
    delay(300);
  }
  Games_Iterator = -1;
  Games_Iterator = Games_Number; // Start over the game
  difficulty = 0; // If the difficulty was set to Hard, we set it to Easy
  game_done = false;
}

void player_input(){ // Functie ajutatoare pentru Memory Game
  for (iterator = 0; iterator <= Games_Iterator;){
    for (iterator_2 = 0; iterator_2 < NumberOfLEDs; iterator_2++){
      buttonstate = analogRead(Buttons[iterator_2]);
    
      if (buttonstate < 100 && Buttons[iterator_2] == 50){ // Daca a fost apasat butonul corespunzator led-ului rosu
        digitalWrite(LEDs[0], HIGH);
        delay(150);
        digitalWrite(LEDs[0], LOW);
        input_array[iterator] = 1;
        delay(150);
        if (input_array[iterator] != random_array[iterator]) { // Daca jucatorul a gresit secventa
          check_fail();                              
        }                              
        iterator++;
      }
       if (buttonstate == LOW && Buttons[iterator_2] == 51){ // Daca a fost apasat butonul corespunzator led-ului verde
        digitalWrite(LEDs[1], HIGH);
        delay(150);
        digitalWrite(LEDs[1], LOW);
        input_array[iterator] = 2;
        delay(150);
        if (input_array[iterator] != random_array[iterator]) { // Daca jucatorul a gresit secventa
          check_fail();                              
        }                              
        iterator++;
      }
      if (buttonstate == LOW && Buttons[iterator_2] == 53){ // Daca a fost apasat butonul corespunzator led-ului albastru
        digitalWrite(LEDs[2], HIGH);
        delay(150);
        digitalWrite(LEDs[2], LOW);
        input_array[iterator] = 3;
        delay(150);
        if (input_array[iterator] != random_array[iterator]) { // Daca jucatorul a gresit secventa
          check_fail();                              
        }                              
        iterator++;
      }
      if (buttonstate == LOW && Buttons[iterator_2] == 52){ // Daca a fost apasat butonul corespunzator led-ului galben
        digitalWrite(LEDs[3], HIGH);
        delay(150);
        digitalWrite(LEDs[3], LOW);
        input_array[iterator] = 4;
        delay(150);
        if (input_array[iterator] != random_array[iterator]) { // Daca jucatorul a gresit secventa
          check_fail();                              
        }                              
        iterator++;
      }
    }
  }
  delay(500);
  Games_Iterator ++;
}

void memory_game(){
    digitalWrite(LEDs[0], HIGH);
    digitalWrite(LEDs[1], HIGH);
    digitalWrite(LEDs[2], HIGH);
    digitalWrite(LEDs[3], HIGH);
    delay(300);
    digitalWrite(LEDs[0], LOW);
    digitalWrite(LEDs[1], LOW);
    digitalWrite(LEDs[2], LOW);
    digitalWrite(LEDs[3], LOW);
    delay(300);
    
  if(difficulty == 0){
    Games_Number = 4;
  }
  else{
    Games_Number = 6;
  }
  Games_Iterator = Games_Number;
  for (int iterator_1 = 0; iterator_1 < Games_Number; iterator_1++){ 
    digitalWrite(LEDs[0], HIGH);
    digitalWrite(LEDs[1], HIGH);
    digitalWrite(LEDs[2], HIGH);
    digitalWrite(LEDs[3], HIGH);
    delay(300);
    digitalWrite(LEDs[0], LOW);
    digitalWrite(LEDs[1], LOW);
    digitalWrite(LEDs[2], LOW);
    digitalWrite(LEDs[3], LOW);
    delay(300);
    for (int iterator_1 = 0; iterator_1 <= Games_Iterator; iterator_1++){
      random_array[iterator_1] = random(1, 5); 
        for( iterator_2 = 0; iterator_2 < Games_Iterator; iterator_2++){
          for(int iterator_1 = 0; iterator_1 < NumberOfLEDs; iterator_1++){
         if (random_array[iterator_2] == 1 && LEDs[iterator_1] == 50) {  //if statements to display the stored values in the array
            digitalWrite(LEDs[0], HIGH);
            delay(400);
            digitalWrite(LEDs[0], LOW);
            delay(100);
          }
         if (random_array[iterator_2] == 2 && LEDs[iterator_1] == 51) {  //if statements to display the stored values in the array
            digitalWrite(LEDs[1], HIGH);
            delay(400);
            digitalWrite(LEDs[1], LOW);
            delay(100);
          }
          if (random_array[iterator_2] == 3 && LEDs[iterator_1] == 53) {  //if statements to display the stored values in the array
            digitalWrite(LEDs[2], HIGH);
            delay(400);
            digitalWrite(LEDs[2], LOW);
            delay(100);
          }
         if (random_array[iterator_2] == 4 && LEDs[iterator_1] == 52) {  //if statements to display the stored values in the array
            digitalWrite(LEDs[3], HIGH);
            delay(400);
            digitalWrite(LEDs[3], LOW);
            delay(100);
          }
        }
      }
    }
    player_input();
  }
  game_done = true;
}

// ##### ALARM MODE 3 - PUSHBUTTON GAME #####

void push_game(){
  if(difficulty == 0){
    Games_Number = random(5, 10);
  }
  else{
    Games_Number = random(10, 31);
  }
 
  buttonstate = analogRead(Button_Game);
  if(buttonstate != last_buttonstate){
    if(buttonstate < 100){
    last_buttonstate = 99;
    Button_counter++;
  }
  delay(25);
  last_buttonstate = buttonstate;
  }
  if(Button_counter >= Games_Number){
    game_done = true;
    Button_counter = 0;
  }
}

void loop() {
   if (currentPage == '0') {
    if(RTC.read(tm)){
      screen.setFont(SevenSeg_XXXL_Num);
      screen.setColor(DarkGreen);
      if ( currentSeconds != tm.Second) {
        screen.printNumI(tm.Second, 328, 50, 2, '0');
        currentSeconds = tm.Second;
      }
      if ( currentMinutes != tm.Minute ) {
      screen.printNumI(tm.Minute, 176, 50, 2, '0');
      currentMinutes = tm.Minute;
      }
      if ( currentHour != tm.Hour ) {
        screen.printNumI(tm.Hour, 24, 50, 2, '0');
        currentHour = tm.Hour;
      }
      
      screen.setFont(BigFont); // Sets font to big
      if ( currentDay != tm.Day ) {
        screen.printNumI(tm.Day, 275, 7, 2, '0');
        currentDay = tm.Day;
      }
      screen.print("/", 310, 7);
      if ( currentMonth != tm.Month ) {
        screen.printNumI(tm.Month, 330, 7, 2, '0');
        currentMonth = tm.Month;
      }
      screen.print("/", 375, 7);
      if ( currentYear != tmYearToCalendar(tm.Year) ) {
        screen.printNumI(tmYearToCalendar(tm.Year), 390, 7, 4, '0');
        currentYear= tmYearToCalendar(tm.Year);
      }
      RTC.read(tm);
    }
   }
   if (touch.dataAvailable() == true){
    int pressed_button = button.checkButtons();
    if(pressed_button == alarm_button){
      currentPage = '1'; // Alarm set page;
      screen.clrScr();
    }
    if(pressed_button == settings_button){ 
      currentPage = '2'; // Settings page;
      screen.clrScr();
      delay(50);
    }
    if(pressed_button == media_button){
      currentPage = '3'; // Media Player Page
      screen.clrScr();
      delay(50);
      drawMediaPlayerScreen();
      delay(50);
    } 
  }
  
   // Media Player Screen
  if (currentPage == '3') {
    if (touch.dataAvailable() == true) {
      pressed_button = button.checkButtons();
    // If we press the Play Button
      if(pressed_button == play_button){
        if (playStatus == '0') {
          button.drawButton(pause_button);
          MP3_Player.wakeUp();
          MP3_Player.playFolder(02, 01);
          delay(50);
          playStatus = '2';
          return;
          }
        if (playStatus == '1') {
          button.drawButton(pause_button);
          MP3_Player.resume();
          delay(50);
          playStatus = '2';
          return;
          }
        if (playStatus == '2') {
          button.drawButton(play_button);
          MP3_Player.pause();
          delay(50);
          playStatus = '1';
          return;
          }
      }

      // If we press the Previous Button
      if(pressed_button == previous_button){
        MP3_Player.playPrevious();
        delay(50);
        }
        
      // If we press the Next Button
      if(pressed_button == next_button){
        MP3_Player.playNext();
        delay(50);
        }

      // If we press the Volume Down Button
      if(pressed_button == volumeDown_button){
        if (iV >= 10 && iV <= 30) {
          iV--;
          MP3_Player.decVolume();
          }
        delay(50);
      }
      
      // If we press the Volume Up Button
      if(pressed_button == volumeDown_button){
        if (iV >= 10 && iV < 30) {
          iV++;
          MP3_Player.incVolume();
          }
        delay(50);
      }
      
      // If we press the HOME Button
      if ((x_cord >= 24) && (x_cord <= 80) && (y_cord >= 7) && (y_cord <= 23)) {
        screen.clrScr();
        drawHomeScreen();  // Draws the Home Screen
        currentPage = '0';
        return;
      }
    }
  } // end of current page = ' 3 '
   
  // Settings Screen
  if (currentPage == '2'){
  screen.clrScr();
  int button_1, button_2, button_3, easy_button, hard_button;
  int alarm1_button, alarm2_button, alarm3_button; 
  screen.setBackColor(Black);
  screen.setColor(White); 
  screen.setFont(BigFont);
  screen.print("HOME", 24, 7);
  screen.print("ALARM MODE: ", CENTER, 145);
  screen.print("MODE: ", 350, 60);
  screen.print("SONGS: ", 62, 60);
  screen.setColor(DarkGreen);
  screen.drawLine(0, 28, 479, 28);

  button.setButtonColors(White, White, DarkGreen, White, DarkGreen);

  button_1 = button.addButton(12, 78, 96, 30, "Song 1");
  button_2 = button.addButton(108, 78, 96, 30, "Song 2");
  button_3 = button.addButton(216, 78, 96 ,30, "Song 3");

  easy_button = button.addButton(324, 78, 66, 30, "Easy");
  hard_button = button.addButton(402, 78, 66, 30, "Hard");
  
  alarm1_button = button.addButton(45, 166, 100, 100, "Math");
  alarm2_button = button.addButton(190, 166, 100, 100, "Memory");
  alarm3_button = button.addButton(335, 166, 100, 100, "Press");

  button.drawButton(button_1);
  button.drawButton(button_2);
  button.drawButton(button_3);

  button.drawButton(easy_button);
  button.drawButton(hard_button);

  button.drawButton(alarm1_button);
  button.drawButton(alarm2_button);
  button.drawButton(alarm3_button);
  
  if (touch.dataAvailable() == true) {
    touch.read();
    x_cord = touch.getX(); // x coordinate where the screen has been touched
    y_cord = touch.getY(); // y coordinate where the screen has been touched
    pressed_button = button.checkButtons();
    if(pressed_button == button_1)
      alarmTrack = 1;
    else{
      if(pressed_button == button_2)
        alarmTrack = 2;
      else{
        if(pressed_button == button_3)
          alarmTrack = 3;
      }
    }
    if(pressed_button == easy_button)
      difficulty = 0;
    else{
      if(pressed_button == hard_button)
        difficulty = 1;
    }
    if(pressed_button == alarm1_button)
      alarm_mode = 1;
    else{
      if(pressed_button == alarm2_button)
        alarm_mode = 2;
      else if(pressed_button == alarm3_button)
        alarm_mode = 3;
    }
     if ((x_cord >= 24) && (x_cord <= 75) && (y_cord >= 7) && (y_cord <= 25)) {
        currentPage = '0';
        screen.clrScr();
        delay(50);
        drawHomeScreen();  // Draws the Home Screen
      }
    }
  }
  
  // Alarm Screen
  if (currentPage == '1') {
    screen.clrScr();
    screen.setFont(BigFont);
    screen.setColor(White);
    screen.print("HOME", 24, 7);
    screen.print("Set Alarm", CENTER, 35);
    
    // Draws a colon between the hours and the minutes
    screen.setFont(SevenSeg_XXXL_Num);
    screen.setColor(DarkGreen);
    screen.fillCircle (165, 85, 5);
    screen.setColor(DarkGreen);
    screen.fillCircle (165, 115, 5);

    screen.setColor(DarkGreen);
    screen.printNumI(alarm_hour, 24, 50, 2, '0');
    screen.printNumI(alarm_minutes, 176, 50, 2, '0');
    
    screen.setColor(White);
    screen.drawRoundRect (24, 160, 152, 180);
    screen.drawRoundRect (176, 160, 304, 180);
    screen.setFont(BigFont);    
    screen.print("H", 80, 163);
    screen.print("M", 232, 163);
    
    screen.drawRoundRect (335, 50 , 410, 70);
    screen.print("SET", 345, 53);
    screen.drawRoundRect (335, 160, 410, 180);
    screen.print("CLEAR", 330, 163);
    
    alarm_set = true;
        
    while (alarm_set){
      if (touch.dataAvailable()) {
        touch.read();
        x_cord = touch.getX(); // x coordinate where the screen has been touched
        y_cord = touch.getY(); // y coordinate where the screen has been touched
        
        //Set hours button
        if ((x_cord >= 24) && (x_cord <= 152) && (y_cord >= 160) && (y_cord <= 180)) {
          drawRectFrame(24, 160, 152, 180);
          alarm_hour++;
          if(alarm_hour >= 24){
            alarm_hour = 0;
          }
          screen.setFont(SevenSeg_XXXL_Num);
          screen.setColor(DarkGreen);
          screen.printNumI(alarm_hour, 24, 50, 2, '0');
        }
        
        // Set minutes buttons
        if ((x_cord >= 176) && (x_cord <= 304) && (y_cord >= 160) && (y_cord <= 180)) {
          drawRectFrame(176, 160, 304, 180);
          alarm_minutes++;
          if(alarm_minutes >= 60){
            alarm_minutes = 0;
          }
          screen.setFont(SevenSeg_XXXL_Num);
          screen.setColor(DarkGreen);
          screen.printNumI(alarm_minutes, 176, 50, 2, '0');
      }
      
      // Set alarm button
      if ((x_cord >= 315) && (x_cord <= 400) && (y_cord >= 50) && (y_cord <= 70)) {
        drawRectFrame(315, 50, 400, 70);
        if (alarm_hour < 10 && alarm_minutes < 10){
          alarmString = "0"+(String)alarm_hour + ":" + "0"+ (String)alarm_minutes + ":" + "00";
        }
        else if (alarm_hour < 10 && alarm_minutes > 9){
          alarmString = "0"+(String)alarm_hour + ":" + (String)alarm_minutes + ":" + "00";
        }
        else if (alarm_hour > 9 && alarm_minutes < 10){
          alarmString = (String)alarm_hour + ":" + "0"+ (String)alarm_minutes + ":" + "00";
        }
        else {
          alarmString = (String)alarm_hour + ":" + (String)alarm_minutes + ":" + "00";
        }
        screen.setFont(BigFont);
        screen.print("Alarm set for:", CENTER, 220);
        screen.print(alarmString, CENTER, 260);
      
      }

      // Clear alarm button
      if ((x_cord >= 315) && (x_cord <= 400) && (y_cord >= 160) && (y_cord <= 180)) {
        drawRectFrame(315, 160, 400, 300);
        alarmString="";
        alarm_minutes = alarm_hour = 0;
        screen.setFont(SevenSeg_XXXL_Num);
        screen.setColor(DarkGreen);
        screen.printNumI(alarm_hour, 24, 50, 2, '0');
        screen.printNumI(alarm_minutes, 176, 50, 2, '0');
        
        screen.setColor(Black);
        screen.fillRect(0, 220, 479, 270); 
      }
      
      // If we press the HOME Button
      if ((x_cord >= 24) && (x_cord <= 75) && (y_cord >= 7) && (y_cord <= 25)) {
        alarm_set = false;
        alarm_minutes = alarm_hour = 0;
        currentPage = '0';
        screen.clrScr();
        drawHomeScreen();  // Draws the Home Screen
      }    
     }
    }
   }
   
    // Alarm activation     
    if (alarm_set == false) {
      if (alarm_minutes == tm.Minute && alarm_hour == tm.Hour){
        screen.clrScr();
        MP3_Player.volume(25);
        if(alarmTrack == 1)
          MP3_Player.playFolder(01, 01);
        else {
          if(alarmTrack == 2)
             MP3_Player.playFolder(01, 02);
          else
             MP3_Player.playFolder(01, 03);
        }     
        delay(50);
        screen.setFont(BigFont);
        screen.setColor(White);
        screen.print("ALARM!", CENTER, 100);
        screen.print(alarmString, CENTER, 120);
        int dismiss_button;
        dismiss_button = button.addButton(24, 150, 456, 180, "DISMISS");
        button.drawButton(dismiss_button);
        boolean alarmOn = true;
        while (alarmOn){
          if (touch.dataAvailable() == true) {
            pressed_button = button.checkButtons();
            if(pressed_button == dismiss_button){
            game_done = true;
            }
          }
          while(game_done){
            if(alarm_mode == 1)
                math_game();
            else{
              if(alarm_mode == 2)
                memory_game();
              else
                push_game(); 
            }
          }
          alarmOn = false;
          alarmString="";
          screen.clrScr();
          MP3_Player.sleep();
          }
          delay(50);
          drawHomeScreen();
          currentPage = '0';
          playStatus = '0';  
          }
        }
   }

void drawHomeScreen() {
  screen.clrScr();
  screen.setBackColor(Black); // Sets the background color of the area where the text will be printed to black
  screen.setColor(DarkGreen); 
  screen.setFont(BigFont);
  if (alarmString == "" ) {
  screen.print("Smart Student Alarm Clock", CENTER, 300);
  }
  else {
    screen.print("Alarm set for: ", 24, 300);
    screen.print(alarmString, 250, 300);
  }  
  // Draw the buttons:
  button.setButtonColors(White, White, DarkGreen, White, DarkGreen);
  alarm_button = button.addButton(45, 170, 100, 100, "Alarm");
  settings_button = button.addButton(190, 170, 100, 100, "Settings");
  media_button = button.addButton(335, 170, 100, 100, "Media");
  button.drawButton(alarm_button);
  button.drawButton(settings_button);
  button.drawButton(media_button);
  // Add the colons :
  drawHomeClock();
}

void drawMediaPlayerScreen() {
  screen.clrScr();
  delay(50);
  screen.setBackColor(LightGrey); 
  screen.setColor(White); 
  screen.setFont(BigFont);
  screen.print("HOME", 24, 7);
  screen.setColor(DarkGreen);
  screen.drawLine(0, 28, 479, 28);

  // Track Bar
  screen.setColor(White);
  screen.fillRect (62, 40, 48 + 224, 480 - 62);
  screen.setFont(SmallFont);
  screen.setColor(White);
  
  // Volume Bar
  screen.setColor(White);
  screen.fillRect (85, 274, 85 + 310, 274 + 20);
  screen.setColor(DarkGreen);
  screen.fillRect (85, 274, 85 + 270, 274 + 20);
  
  button.setButtonColors(Black, White,Black , DarkGreen, White);
  
  play_button = button.addButton(190, 120, 100, 100, "PLAY"); 
  pause_button = button.addButton(190, 120, 100, 100, "PAUSE");
  volumeDown_button = button.addButton(30, 280, 32, 32, "V-");
  volumeUp_button = button.addButton(400, 280, 32, 32, "V+");
  previous_button = button.addButton(62, 138, 64, 64, "P");
  next_button = button.addButton(354, 138, 64, 64, "N");
  button.drawButton(pause_button);
  if (playStatus == '2') {
    button.drawButton(pause_button);
  }
  button.drawButton(previous_button);
  button.drawButton(next_button);
  button.drawButton(volumeUp_button);
  button.drawButton(volumeDown_button);
  
}

// Functions to highlight the button when pressed
void drawFrame(int x, int y, int r) {
  screen.setColor(DarkGreen);
  screen.drawCircle (x, y, r);
  while (touch.dataAvailable())
  touch.read();
  screen.setColor(Black);
  screen.drawCircle (x, y, r);
}

void drawRectFrame(int x1, int y1, int x2, int y2) {
  screen.setColor(DarkGreen);
  screen.drawRoundRect (x1, y1, x2, y2);
  while (touch.dataAvailable())
  touch.read();
  screen.setColor(White);
  screen.drawRoundRect (x1, y1, x2, y2);
}


// Sound bar
void drawVolume(int x) {
  screen.setColor(White);
  screen.fillRect (85 + 5 * x, 274, 85 + 310, 274 + 20);
  screen.setColor(DarkGreen);
  screen.fillRect (85, 274, 85 + 5 * x, 274 + 20);
}

void drawColon(){
   screen.setColor(DarkGreen);
  screen.fillCircle (165, 85, 5);
  screen.setColor(DarkGreen);
  screen.fillCircle (165, 115, 5);
  screen.setColor(DarkGreen);
  screen.fillCircle (315, 85, 5);
  screen.setColor(DarkGreen);
  screen.fillCircle (315, 115, 5); 
}
void drawHomeClock() {
  RTC.read(tm);
  screen.setFont(SevenSeg_XXXL_Num);
  screen.setColor(DarkGreen);
  screen.printNumI(tm.Second, 328, 50, 2, '0');
  screen.printNumI(tm.Minute, 176, 50, 2, '0');
  screen.printNumI(tm.Hour, 24, 50, 2, '0');
  screen.setFont(BigFont); // Sets font to big
  drawColon();
}
