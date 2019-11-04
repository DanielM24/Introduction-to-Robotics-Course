const int redPin = 11;
const int greenPin = 10;
const int bluePin = 9;

const int potPin_r = A0;
const int potPin_g = A1;
const int potPin_b = A2;

int redPot = 0;
int greenPot = 0;
int bluePot = 0;

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(potPin_r, INPUT);
  pinMode(potPin_g, INPUT);
  pinMode(potPin_b, INPUT);
  Serial.begin(9600);
}

void loop() {
  redPot = analogRead(potPin_r);
  greenPot = analogRead(potPin_g);
  bluePot = analogRead(potPin_b);

  redValue = map(redPot, 0, 1023, 0, 255);
  greenValue = map(greenPot, 0, 1023, 0, 255);
  blueValue = map(bluePot, 0, 1023, 0, 255);

  ledControl(redValue, greenValue, blueValue);
}

void ledControl(int red, int green, int blue){
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
