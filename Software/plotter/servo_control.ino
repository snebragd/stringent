#include "MachineDefs.h"

#include "MachineDefs.h"
#include <Servo.h>  
Servo myservo;

//disable to preserve space
#define USE_SMOOTH_SERVO

static int oldPos = 0;

void setupServo() 
{
  myservo.attach(SERVO_PIN);  
  movePen(false, true);  
  makePenNoise(2);
}

void makePenNoise(int n)
{
  #ifdef SERIAL_DEBUG
  Serial.println("PEN NOISE");
  #endif
  int p = oldPos;  
  for(int i=0; i<n; i++) {
    myservo.write((int)p+5);    
    delay(100); 
    myservo.write((int)p);    
    delay(100); 
  }  
}

void testPen()
{
  
  Serial.println("PEN TEST");
    movePen(true, false);
    delay(1000); 
    movePen(false, false);
}

void movePen(boolean down, boolean fast)
{ 
  
  Serial.print("PEN MOVE");
  int pos = down ? PEN_DOWN : PEN_UP;

  if(pos != oldPos) {
    Serial.println(pos);

#ifdef USE_SMOOTH_SERVO    
    if(fast || !down) {
        myservo.write((int)pos);
    }
    else {
      for(float i=0 ; i<=1.0 ; i+=0.02) {
        float i2 = (1-cos(i*3.14))/2;
        float tmpPos = (oldPos*(1.0-i2)+(pos*i2));
        myservo.write((int)tmpPos);
        delayMicroseconds(10000); 
      }
    }
#else //USE_SMOOTH_SERVO
    myservo.write((int)pos);
#endif //USE_SMOOTH_SERVO

    oldPos = pos;
  }
}
