#include <Servo.h> 

//disable to preserve space
//#define USE_SMOOTH_SERVO

Servo myservo;

#define UP 100
#define DOWN 20

#define SERVO_PIN 0

static int oldPos = 0;

void setupServo() 
{
#ifndef SERIAL_DEBUG  
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object 
  movePen(false);  
#endif  
}

void movePen(boolean down)
{
#ifndef SERIAL_DEBUG  
  int pos = down ? DOWN : UP;
  if(pos != oldPos) {
#ifdef USE_SMOOTH_SERVO    
    for(float i=0 ; i<=1.0 ; i+=0.01) {
      float i2 = (1-cos(i*3.14))/2;
      float tmpPos = (oldPos*(1.0-i2)+(pos*i2));
      myservo.write((int)tmpPos);
      delay(10);
    }
#else
    myservo.write((int)pos);
#endif
    oldPos = pos;
  }
#endif
}

