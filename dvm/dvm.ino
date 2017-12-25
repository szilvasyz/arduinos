#define RES0 1000000
#define RNG_PINS 4
#define RANGES 4

#define ANA_COM A0
#define ANA_VIN A1

#define DATA_RATE 30

#define VREF_MV 1080L
#define VREF_DIV (VREF_MV * 1024L)

#define RANGE_UP_MARGIN 400
#define RANGE_DN_MARGIN 100


const int RNG_PIN[RNG_PINS] = {10,11,16,17};
const long RNG_RES[RNG_PINS] = {820000,47000,15600,180000};
const int RNG_PAT[RANGES] = {0b0001,0b1001,0b0011,0b0101};
const int RNG_DEC[RANGES] = {2,2,1,1};

double RNG_DIV[RANGES];

int range;
double range_rounding;
int range_decimals;

int cycle = 0;

/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 2 -a 1.0000000000e-01 0.0000000000e+00 -l */

#define NZEROS 2
#define NPOLES 2
#define GAIN   6.701159077e+02

static double xv_val[NZEROS+1], yv_val[NPOLES+1];
static double xv_vcc[NZEROS+1], yv_vcc[NPOLES+1];

double filterloop(double input, double xv[], double yv[]) {
  xv[0] = xv[1]; xv[1] = xv[2]; 
  xv[2] = input / GAIN;
  yv[0] = yv[1]; yv[1] = yv[2]; 
  yv[2] =   (xv[0] + xv[2]) + 2 * xv[1]
               + ( -0.8706834551 * yv[0]) + (  1.8647143385 * yv[1]);
  return yv[2];
}



long readVcc() {
  long result; // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL; result |= ADCH << 8; result = VREF_DIV / result; // Back-calculate AVcc in mV 
  return result;
}


void switchRange(int r) {
  int i;
  double d = 1.0;

  if ((r<0) || (r>=RANGES))
    return;

  for (i=0; i<RNG_DEC[r]; i++) d *= 10.0;
  range_rounding = d;
  range_decimals = RNG_DEC[r];
  range = r;
  
  for (i=0; i<RNG_PINS; i++) {
    digitalWrite( RNG_PIN[i], ((RNG_PAT[r] & (1<<i)) != 0) ? HIGH : LOW );
    Serial.print( RNG_PIN[i] );
    Serial.print(":");
    Serial.print( ((RNG_PAT[r] & (1<<i)) != 0) ? HIGH : LOW );
    Serial.print(" ");
  }
  Serial.println(" ");
}


void setup() {
  // put your setup code here, to run once:
  int i,j;
  double d;

  for (i=0; i<RNG_PINS; i++) {
    pinMode(RNG_PIN[i], OUTPUT);
    digitalWrite(RNG_PIN[i], LOW);
  }
  pinMode(ANA_COM, INPUT);
  pinMode(ANA_VIN, INPUT);

  Serial.begin(9600);
  Serial.println("Hello World!");

  for (i=0; i<RANGES; i++) {
    d = 0;
    for (j=0; j<RNG_PINS; j++) {
      if ((RNG_PAT[i] & (1<<j)) != 0) d += 1.0 / RNG_RES[j];
    }
    if (d != 0) {
      d = 1.0/d;
      RNG_DIV[i] = (d + RES0) / d;
    } else
      RNG_DIV[i] = 1.0;
    
    Serial.print(i);
    Serial.print(": ");
    Serial.println(RNG_DIV[i]);
  }
  
  switchRange(0);

  for (i=0; i<100; i++) {
    filterloop(readVcc(), xv_vcc, yv_vcc);
    filterloop(0.0, xv_val, yv_val);
  }

}



void loop() {
  // put your main code here, to run repeatedly:
  int i;
  static double com, vin, vcc;
  double val;
  byte b;


  if (Serial.available()) {
    switchRange(Serial.read() - '0');
  }

  Serial.flush();
  delay(10);
  
  vin = analogRead(ANA_VIN);
  com = analogRead(ANA_COM);
  vcc = round(filterloop(readVcc(), xv_vcc, yv_vcc) / 10.0) / 100.0;

  val = (vin - com) * RNG_DIV[range];
  val *= vcc / 1024.0;
  val = filterloop(val, xv_val, yv_val);

  
  val = round(range_rounding * val) / range_rounding;

  if (++cycle == DATA_RATE) {
    
    cycle = 0;
    Serial.print(range);
    Serial.print(" ");
    Serial.print(RNG_DIV[range]);
    Serial.print(" ");
    Serial.print(vcc);
    Serial.print(" ");
    Serial.print(vcc*RNG_DIV[range]/2.0);
    Serial.print(" ");
    Serial.print(com);
    Serial.print(" ");
    Serial.print(vin);
    Serial.print(": ");
    Serial.println(val,range_decimals);
    
    if ((abs(vin-com) > RANGE_UP_MARGIN) && (range < (RANGES-1))) {
      switchRange(++range);
    }
    
    if ((abs(vin-com) < RANGE_DN_MARGIN) && (range > 0)) {
      switchRange(--range);
    }
  }

}

