#include "MachineDefs.h"

#include "MachineDefs.h"
#include <Servo.h>  
Servo myservo;

//disable to preserve space
#define USE_SMOOTH_SERVO
//#define USE_LINEAR_SERVO

static int oldPos = 0;

void setupServo() 
{
  myservo.attach(SERVO_PIN);  
  movePen(false);  
}

void makePenNoise()
{
  int p = oldPos;  
  for(int i=0;i<4;i++) {
    myservo.write((int)p-15);    
    delay(400000); 
    myservo.write((int)p);    
    delay(400000); 
  }  
}

void movePen(boolean down)
{
  int pos = down ? PEN_DOWN : PEN_UP;
  if(pos != oldPos) {

#ifdef USE_SMOOTH_SERVO    

#ifdef USE_LINEAR_SERVO    
    while(pos != oldPos) {
      myservo.write(oldPos);
      oldPos = oldPos + ((pos-oldPos) > 0 ? 1 : -1);
       delayMicroseconds(5000); 
    }
#else //USE_LINEAR_SERVO    
    for(float i=0 ; i<=1.0 ; i+=0.01) {
      float i2 = (1-cos(i*3.14))/2;
      float tmpPos = (oldPos*(1.0-i2)+(pos*i2));
      myservo.write((int)tmpPos);
      delayMicroseconds(10000); 
    }
#endif //USE_LINEAR_SERVO

    oldPos = pos;
#else //USE_SMOOTH_SERVO
    myservo.write((int)pos);
    oldPos = pos;
#endif //USE_SMOOTH_SERVO
  }
}

