
#define SERIAL_DEBUG
//#define NO_REMOTE

#ifdef SERIAL_DEBUG
#define SER_PRINT(X) Serial.print((X))
#define SER_PRINT2(X,Y) Serial.print((X),(Y))
#define SER_PRINTLN(X) Serial.println((X))
#define SER_PRINTLN2(X,Y) Serial.println((X),(Y))
#else
#define SER_PRINT(X)
#define SER_PRINT2(X,Y)
#define SER_PRINTLN(X)
#define SER_PRINTLN2(X,Y)
#endif

//define this if plotter has a battery voltage feedback connected
//#define HAS_BATTERY_MEASUREMENT 
 
//which remote control to use
//#define PIONEER_DVD_REMOTE
//#define NONAME_WHITE_REMOTE
#define NODEMCU_WEBSERVER

//servo pin
#define SERVO_PIN D0

//servo values for pen up/down
#define PEN_UP 80
#define PEN_DOWN 60


//IR-receiver pin
//#define RECV_PIN  A0
 
//stepper motor pins
#define LEFT_STEP_PIN_1  D5 // 6
#define LEFT_STEP_PIN_2  D6 // 7
#define LEFT_STEP_PIN_3 D7 // 8
#define LEFT_STEP_PIN_4 D8 // 9
#define RIGHT_STEP_PIN_1 D1 // 2
#define RIGHT_STEP_PIN_2 D2 // 3
#define RIGHT_STEP_PIN_3 D3 // 4
#define RIGHT_STEP_PIN_4 D4 // 5
