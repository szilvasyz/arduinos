#include <LiquidCrystal.h>


// keyboard connected to analog pin
#define KBD A0

// discharge control outputs
#define C1_DC 20
#define C2_DC 21

// charge control outputs (negated!)
#define C1_CH 18
#define C2_CH 19

// lower point of discharging resistors
#define C1_LO A8
#define C2_LO A9

// upper point of discharging restistors
#define C1_HI A10
#define C2_HI A11

// power point of charging resistors
#define C1_PW A12
#define C2_PW A13

// values of discharging resistors
#define C1_DCR 10.0
#define C2_DCR 10.0

// values of charging resistors
#define C1_CHR 10.0
#define C2_CHR 10.0

// Vref = supply voltage, Vmin is the discharging voltage limit, Imin is charging current limit
#define VREF 5.00
#define VMIN 2.70
#define IMIN 0.10

// resistors of voltage divider on charging power line
#define PW_RU 39000.0
#define PW_RL 33000.0

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define RS 8
#define EN 9
#define DB4 4
#define DB5 5
#define DB6 6
#define DB7 7

LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);


void setup() {

  Serial.begin(9600);
  
  pinMode(C1_DC,OUTPUT); digitalWrite(C1_DC,LOW);
  pinMode(C2_DC,OUTPUT); digitalWrite(C2_DC,LOW);
  
  pinMode(C1_CH,OUTPUT); digitalWrite(C1_CH,HIGH);
  pinMode(C2_CH,OUTPUT); digitalWrite(C2_CH,HIGH);

  pinMode(C1_LO,INPUT);
  pinMode(C2_LO,INPUT);
  pinMode(C1_HI,INPUT);
  pinMode(C2_HI,INPUT);
  pinMode(C1_PW,INPUT);
  pinMode(C2_PW,INPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Hello, world!");
}


char* format(unsigned long u, int n) {
  static char b[20];
  unsigned long d = 1;
  int i;

  for (i=1; i<n; i++) d *= 10;
  for (i=0; i<n; i++) {
    b[i] = '0' + (u/d);
    u %= d;
    d /= 10;
  }
  b[i] = '\0';
  return b;
}


int hi,lo;
unsigned long mil;
float c1_cv,c1_di,c1_ci;
float c2_cv,c2_di,c2_ci;
float c1_cap = 0.0;
float c2_cap = 0.0;

bool c1_dc = false;
bool c2_dc = false;

bool c1_ch = false;
bool c2_ch = false;

unsigned long c1_mil;
unsigned long c2_mil;

float v1,v2;
int i1,i2;
int c1,c2;

int k;


void loop() {

  hi = analogRead(C1_HI);
  lo = analogRead(C1_LO);

  // cell voltage
  c1_cv = hi * VREF / 1023.0;

  // discharge current
  c1_di = (hi - lo) * VREF / 1023.0 / C1_DCR;
  if (c1_di < 0.0) c1_di = 0.0;

  lo = hi;
  hi = analogRead(C1_PW) * (PW_RU+PW_RL) / PW_RL;

  // charge current
  c1_ci = (hi - lo) * VREF / 1023.0 / C1_CHR;
  if (c1_ci < 0.0) c1_ci = 0.0;

  Serial.print(c1_cv); Serial.print(" ");
  Serial.print(c1_ci); Serial.print(" ");
  Serial.print(c1_di); Serial.print(" ");
  Serial.print(c1_cap); Serial.print(" ");
  Serial.print(" - ");

  
  hi = analogRead(C2_HI);
  lo = analogRead(C2_LO);

  // cell voltage
  c2_cv = hi * VREF / 1023.0;

  // discharge current
  c2_di = (hi - lo) * VREF / 1023.0 / C2_DCR;
  if (c2_di < 0.0) c2_di = 0.0;

  lo = hi;
  hi = analogRead(C2_PW) * (PW_RU+PW_RL) / PW_RL;

  // charge current
  c2_ci = (hi - lo) * VREF / 1023.0 / C2_CHR;
  if (c2_ci < 0.0) c2_ci = 0.0;

  Serial.print(c2_cv); Serial.print(" ");
  Serial.print(c2_ci); Serial.print(" ");
  Serial.print(c2_di); Serial.print(" ");
  Serial.print(c2_cap); Serial.print(" ");
  Serial.println();


  mil = millis();


  // discharge stop
  if (c1_cv < VMIN) {
    c1_dc = false;
    digitalWrite(C1_DC,LOW);
  }

  if (c2_cv < VMIN) {
    c2_dc = false;
    digitalWrite(C2_DC,LOW);
  }


  // charge stop
  if (c1_ci < IMIN) {
    c1_ch = false;
    digitalWrite(C1_CH,HIGH);
  }

  if (c2_ci < IMIN) {
    c2_ch = false;
    digitalWrite(C2_CH,HIGH);
  }


  if (( k = analogRead(KBD)) < 800) {
    c1_cap = 0.0;
    c2_cap = 0.0;

    c1_mil = mil;
    c2_mil = mil;

    if (k<500) {
      c1_dc = true;
      c2_dc = true;
      c1_ch = false;
      c2_ch = false;
      digitalWrite(C1_DC,HIGH);
      digitalWrite(C1_CH,HIGH);
      digitalWrite(C2_DC,HIGH);
      digitalWrite(C2_CH,HIGH);
    }
    else {
      c1_ch = true;
      c2_ch = true;
      c1_dc = false;
      c2_dc = false;
      digitalWrite(C1_DC,LOW);
      digitalWrite(C1_CH,LOW);
      digitalWrite(C2_DC,LOW);
      digitalWrite(C2_CH,LOW);
    }
  }

// discharging
  if (c1_dc) {
    c1_cap += (c1_di * (mil - c1_mil)) / 3600.0;
    c1_mil = mil;
  }

  if (c2_dc) {
    c2_cap += (c2_di * (mil - c2_mil)) / 3600.0;
    c2_mil = mil;
  }


// charging
  if (c1_ch) {
    c1_cap += (c1_ci * (mil - c1_mil)) / 3600.0;
    c1_mil = mil;
  }

  if (c2_ch) {
    c2_cap += (c2_ci * (mil - c2_mil)) / 3600.0;
    c2_mil = mil;
  }


  v1 = c1_cv;
  v2 = c2_cv;

  i1 = 1000.0 * ( c1_dc ? c1_di : c1_ci );
  i2 = 1000.0 * ( c2_dc ? c2_di : c2_ci );

  c1 = c1_cap;
  c2 = c2_cap;

  lcd.setCursor(0, 0);
  lcd.print(c1_ch ? 'C' : (c1_dc ? 'D' : ' '));
  lcd.print(v1);
  lcd.print(" ");
  lcd.print(format(i1,3));
  lcd.print(" ");
  lcd.print(format(c1,5));

  lcd.setCursor(0, 1);
  lcd.print(c2_ch ? 'C' : (c2_dc ? 'D' : ' '));
  lcd.print(v2);
  lcd.print(" ");
  lcd.print(format(i2,3));
  lcd.print(" ");
  lcd.print(format(c2,5));

  delay(500);
  
}
