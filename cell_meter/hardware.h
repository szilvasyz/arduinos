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



#ifdef ARDUINO_AVR_MEGA2560

// keyboard connected to analog pin
#define KBD A0
#define KBD_INIT() { pinMode(A0,INPUT); }
#define KBD_READ() ( int k=analogRead(A0) > 800 ? 0 : (k < 500 ? 2 : 1) )

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


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define RS 8
#define EN 9
#define DB4 4
#define DB5 5
#define DB6 6
#define DB7 7


#endif


#ifdef ARDUINO_AVR_PRO

// keyboard connected to 2 digital pins
#define KBD_INIT() { pinMode(2,INPUT_PULLUP); pinMode(3,INPUT_PULLUP); }
#define KBD_READ() ( digitalRead(2) == LOW ? 1 : (digitalRead(3) == 0 ? 2 : 0) )

// discharge control outputs
#define C1_DC 10
#define C2_DC 11

// charge control outputs (negated!)
#define C1_CH 13
#define C2_CH 12

// lower point of discharging resistors
#define C1_LO A0
#define C2_LO A1

// upper point of discharging restistors
#define C1_HI A2
#define C2_HI A3

// power point of charging resistors
#define C1_PW A5
#define C2_PW A4

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define RS 4
#define EN 5
#define DB4 6
#define DB5 7
#define DB6 8
#define DB7 9

#endif

