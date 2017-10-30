#include <LiquidCrystal.h>

#define KBD A0

#define C1_ON 20
#define C2_ON 21
#define C1_LO A8
#define C2_LO A9
#define C1_HI A10
#define C2_HI A11

#define R1 10.0
#define R2 10.0

#define VREF 5.00
#define VMIN 2.50


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {

  pinMode(C1_ON,OUTPUT); digitalWrite(C1_ON,LOW);
  pinMode(C2_ON,OUTPUT); digitalWrite(C2_ON,LOW);
  pinMode(C1_LO,INPUT);
  pinMode(C2_LO,INPUT);
  pinMode(C1_HI,INPUT);
  pinMode(C2_HI,INPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Hello, world!");
}


int hi,lo;
unsigned long mil;
float v1,i1;
float v2,i2;
float c1_cap = 0.0;
bool c1_run = false;
unsigned long c1_mil;
float c2_cap = 0.0;
bool c2_run = false;
unsigned long c2_mil;


void loop() {

  hi = analogRead(C1_HI);
  lo = analogRead(C1_LO);
  
  v1 = hi * VREF / 1023.0;
  i1 = (hi - lo) * VREF / 1023.0 / R1;
  if (i1 < 0.0) i1 = 0.0;
  
  hi = analogRead(C2_HI);
  lo = analogRead(C2_LO);
  
  v2 = hi * VREF / 1023.0;
  i2 = (hi - lo) * VREF / 1023.0 / R2;
  if (i2 < 0.0) i2 = 0.0;

  mil = millis();

    
  if (v1 < VMIN) c1_run = false;
  if (v2 < VMIN) c2_run = false;

  if (analogRead(KBD) < 800) {
    c1_run = true;
    c1_cap = 0.0;
    c1_mil = mil;
    c2_run = true;
    c2_cap = 0.0;
    c2_mil = mil;
  }

  if (c1_run) {
    digitalWrite(C1_ON,HIGH);
    
    c1_cap += (i1 * (mil - c1_mil)) / 3600.0;
    c1_mil = mil;
  }
  else {
    digitalWrite(C1_ON,LOW);
  }

  if (c2_run) {
    digitalWrite(C2_ON,HIGH);
    c2_cap += (i2 * (mil - c2_mil)) / 3600.0;
    c2_mil = mil;
  }
  else {
    digitalWrite(C2_ON,LOW);
  }


  lcd.setCursor(0, 0);
  lcd.print(v1);
  lcd.print(" ");
  lcd.print(i1);
  lcd.print(" ");
  lcd.print(c1_cap);

  lcd.setCursor(0, 1);
  lcd.print(v2);
  lcd.print(" ");
  lcd.print(i2);
  lcd.print(" ");
  lcd.print(c2_cap);

  delay(100);
  
}
