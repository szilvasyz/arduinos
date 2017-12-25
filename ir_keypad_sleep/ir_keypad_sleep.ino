#include <PinChangeInterrupt.h>
#include "LowPower.h"


#define ROWS 5
#define COLS 4
#define NO_KEY 0x7F

#define KCNT_PRS 2
#define KCNT_REP 50
#define KCNT_MAX 52


// IR xmit defines

#define IR_LED 11


#define IR_PORT digitalPinToPort(IR_LED)
#define IR_BIT digitalPinToBitMask(IR_LED)
#define IR_OUT portOutputRegister(IR_PORT)

#define IR_TCCRnA TCCR1A
#define IR_TCCRnB TCCR1B
#define IR_TCNTn TCNT1
#define IR_TIFRn TIFR1
#define IR_TIMSKn TIMSK1
#define IR_TOIEn TOIE1
#define IR_OCIEn OCIE1A
#define IR_ICRn ICR1
#define IR_OCRn OCR1A

#define IR_COMn0 COM1B0
#define IR_COMn1 COM1B1

#define PRONTO_IR_SOURCE 0 // Pronto code byte 0
#define PRONTO_FREQ_CODE 1 // Pronto code byte 1
#define PRONTO_SEQUENCE1_LENGTH 2 // Pronto code byte 2
#define PRONTO_SEQUENCE2_LENGTH 3 // Pronto code byte 3
#define PRONTO_CODE_START 4 // Pronto code byte 4



//define the symbols on the buttons of the keypads
//char myKeys[ROWS][COLS] = {
//  {'F','G','#','*'},
//  {'1','2','3','^'},
//  {'4','5','6','v'},
//  {'7','8','9','E'},
//  {'<','0','>','R'}
//};
char myKeys[ROWS][COLS] = {
  { 0, 1, 2, 3},
  { 4, 5, 6, 7},
  { 8, 9,10,11},
  {12,13,14,15},
  {16,17,18,19}
};

char myChrs[] = {
  'F','G','#','*',
  '1','2','3','^',
  '4','5','6','v',
  '7','8','9','E',
  '<','0','>','R'
  };

//byte rowPins[ROWS] = {10, 9, 8, 7, 6};
//byte colPins[COLS] = {2, 3, 4, 5};

byte rowPins[ROWS] = {2, 3, 4, 5, 6};
byte colPins[COLS] = {10, 9, 8, 7};

char key;
int keyCounter = 0;


// variables for IR xmit
volatile uint16_t *ir_code;
volatile uint16_t  ir_count;
volatile uint8_t   ir_output;
volatile uint16_t  ir_pointer;


// AUX VID CD  TUN
// PHN *2* *3* V+
// *4* *5* *6* V-
// *7* *8* *9* TAP
// --- --- --- ---

// RA1145R
// NEC2 81 72
//
// 48 - TAPE
// 44 - TUNER
// 45 - AUX
// 49 - CD
// 4A - VIDEO
// 4B - ?
// 4C - PHONO
// 4D - VID2
// 5E - DOWN
// 5F - UP

byte irDev[] = {0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81};
byte irSub[] = {0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72};
byte irObc[] = {0x45,0x4a,0x49,0x44,0x4c,0x00,0x00,0x5f,0x00,0x00,0x00,0x5e,0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x00};

uint16_t array[80];



void pcInt() {
  
}


ISR(TIMER1_OVF_vect) {
  if (ir_output != 0) *IR_OUT |= IR_BIT;
}


ISR(TIMER1_COMPA_vect) {
  *IR_OUT &= ~IR_BIT;
 
  if (ir_count>0) {
    ir_count--;
 
    if (ir_count==0) {
      ir_output = 1-ir_output;
   
      if (ir_pointer != 0) {
        ir_count = ir_code[ir_pointer];
        ir_pointer++;

        if (ir_count==0) {
          ir_pointer = 0;
          ir_output = 0;
        }
      }
    }
  }
}



void ir_send(uint16_t *code) {

  ir_output = 0;
  ir_count = 0;
  ir_pointer = 0;

  ir_code = code;

  IR_TCCRnA = 0x00; // Reset the pwm
  IR_TCCRnB = 0x00;
 
  uint16_t top = ( (F_CPU/1000000.0) * code[PRONTO_FREQ_CODE] * 0.241246 ) - 1;

  // timer counts to TOP, output compare interrupt on the half
  IR_ICRn = top;
  IR_OCRn = top >> 1;

  // Fast PWM; top=ICR
  IR_TCCRnA = (1<<WGM11);
  IR_TCCRnB = (1<<WGM13) | (1<<WGM12);

  // reset counter
  IR_TCNTn = 0x0000;

  // clear interrupts
  IR_TIFRn = 0x00;

  // timer overflow and compare interrupt enable
  IR_TIMSKn = (1 << IR_TOIEn) | (1 << IR_OCIEn);
 
  // prescaler=1
  IR_TCCRnB |= (1<<CS10);

  ir_pointer = PRONTO_CODE_START;

  // start xmit with some "space"
  ir_count = 10;
  
  while (ir_pointer != 0);

}


uint16_t generate_nec2(uint16_t *code, uint16_t device, uint16_t subdevice, uint16_t obc) {

  uint16_t i,j;
  uint16_t nobc;

  j = 0;
  code[j++] = 0;
  code[j++]= 0x6c;
  code[j++]= 0;
  code[j++]= 0x22;
  code[j++]= 0x156;
  code[j++]= 0xAA;

  nobc = ~obc;

  for (i=0; i<8; i++) { code[j++]= 0x15; code[j++]= ((device & 1) == 0) ? 0x15 : 0x40; device >>= 1; }
  for (i=0; i<8; i++) { code[j++]= 0x15; code[j++]= ((subdevice & 1) == 0) ? 0x15 : 0x40; subdevice >>= 1; }
  for (i=0; i<8; i++) { code[j++]= 0x15; code[j++]= ((obc & 1) == 0) ? 0x15 : 0x40; obc >>= 1; }
  for (i=0; i<8; i++) { code[j++]= 0x15; code[j++]= ((nobc & 1) == 0) ? 0x15 : 0x40; nobc >>= 1; }

  code[j++]= 0x16;
  code[j++]= 0x614;

  code[j++]= 0;

  return j;
}





void initKpd(byte *rowPins, byte *colPins) {
  byte i;

  for (i = 0; i < ROWS; i++) {
    pinMode(rowPins[i],OUTPUT);
    digitalWrite(rowPins[i],LOW);
  }
  for (i = 0; i < COLS; i++) {
    pinMode(colPins[i],INPUT_PULLUP);
    attachPCINT(digitalPinToPCINT(colPins[i]), pcInt, LOW);
  }

}

char readKpd(char keyMap[ROWS][COLS]) {
  byte r;
  byte c;
  int rp;
  int cp;

  rp = -1;
  cp = -1;
  
  for (r = 0; r < ROWS; r++) {
    pinMode(rowPins[r],INPUT_PULLUP);
  }

  for (r = 0; r < ROWS; r++) {
    pinMode(rowPins[r],OUTPUT);
    digitalWrite(rowPins[r],LOW);
    for (c=0; c < COLS; c++) {
      if (digitalRead(colPins[c]) == LOW) { rp = r; cp = c; }
    }
    pinMode(rowPins[r],INPUT_PULLUP);
  }

  for (r = 0; r < ROWS; r++) {
    pinMode(rowPins[r],OUTPUT);
    digitalWrite(rowPins[r],LOW);
  }

  if (rp >= 0)
    return keyMap[rp][cp];
  else
    return NO_KEY;
}



void setup() {
  // put your setup code here, to run once:
  initKpd(rowPins, colPins);
  //Serial.begin(9600);
  pinMode(0,INPUT);
  pinMode(1,INPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(IR_LED,OUTPUT);
}


void processKey(char key) {
  if (irObc[key] != 0) {
    digitalWrite(LED_BUILTIN,HIGH);
//    Serial.begin(9600);
//    Serial.println(myChrs[key]);
//    Serial.end();

    generate_nec2(array,irDev[key],irSub[key],irObc[key]);
    ir_send(array);
    digitalWrite(LED_BUILTIN,LOW);
  }
}





void loop() {
  while ((key = readKpd(myKeys)) != NO_KEY) {
    if (keyCounter == KCNT_MAX) keyCounter = KCNT_REP;
    if ((keyCounter == KCNT_PRS) || (keyCounter == KCNT_REP)) processKey(key);
    keyCounter++;
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
  }

  keyCounter = 0;
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
