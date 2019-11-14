const int knockPin = A0;
const int speakerPin = 10;
const int buttonPin = 12;

int buttonValue;
int knockValue = 0;
int iterator = 0;

const int knockThreshold = 3;
const int songDelay = 5000;

unsigned long toneDelay = 5;
unsigned long lastKnockTime = 0;

bool playMusic = false;

void setup() {
  pinMode(knockPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void knockDetector() {
  int knockValue = analogRead(knockPin);
  if (knockValue >= knockThreshold) {
    playMusic = true;
    lastKnockTime = millis();
    Serial.println("Knock Knock");
  }
}

void playSong() {
  if((millis() - lastKnockTime) >= songDelay){
    for (iterator = 440; iterator < 1000; iterator++) {
        if((millis() - lastKnockTime) > toneDelay) {
            tone(speakerPin, iterator, 3);
            toneDelay += 5;
       }
   }
     for (iterator = 1000; iterator > 440; iterator--) {
        if ((millis() - lastKnockTime) > toneDelay) {
            tone(speakerPin, iterator, 3);
            toneDelay += 5;
       }
   } 
  }
  toneDelay = 5;
}

void stopSong() {
  buttonValue = digitalRead(buttonPin);
  if (buttonValue == LOW) {
    playMusic = false;
    noTone(speakerPin);
  }
}
void loop() {
  if (playMusic == false) {
    knockDetector();  
  }
  else {
    playSong();
    stopSong();
  }
}
