
long currLeftPos = 0;
long currRightPos = 0;

byte stepSequence[8] = {B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001};

byte leftPins[4] = {6,7,8,9};
byte rightPins[4] = {2,3,4,5};

#define MIN_DELAY 1220
#define MAX_DELAY 20000
#define MAX_ACCEL ((MAX_DELAY-MIN_DELAY)/((maxSegmentLength/2)*stepsPerMM))

float delayMicros = MIN_DELAY;

//disable steppers after half a second of inactivity
#define DISABLE_TIMEOUT 500000
unsigned long lastStepChange = 0;

void setupStep() 
{
    for(int pin=0 ; pin<4 ; pin++) {
      pinMode(leftPins[pin], OUTPUT);
      pinMode(rightPins[pin], OUTPUT);
    }
}

void step(long leftSteps, long rightSteps, float accStart, float accStop)
{
  long numSteps = max(abs(leftSteps), abs(rightSteps));
  long startBreakingAt = numSteps;

  delayMicros = (accStart > 0) ? MAX_DELAY : MIN_DELAY;

  if(accStop > 0) {    
    startBreakingAt = numSteps-(MAX_DELAY-MIN_DELAY)/MAX_ACCEL;    
  }
  
  if(numSteps > 0) {
     float leftPerStep = abs(leftSteps) / (float)numSteps;
     float rightPerStep = abs(rightSteps) / (float)numSteps;     
     float leftFraction = 0;
     float rightFraction = 0;
    
/*     Serial.print("numsteps=");
     Serial.println(numSteps);     
     Serial.print("dL=");
     Serial.println(leftPerStep);     
     Serial.print("dR=");
     Serial.println(rightPerStep);     
  */  
     while(abs(leftSteps) > 0 || abs(rightSteps) > 0) {
       leftFraction += leftPerStep;
       if(leftFraction >= 1.0) { //step left
         leftFraction -= 1.0;        

         if(leftSteps > 0) {
           leftSteps--;
           currLeftPos++;
         }
         else if(leftSteps < 0) {
           leftSteps++;
           currLeftPos--;
         }
         byte mask = stepSequence[currLeftPos & 0x7];
         digitalWrite(leftPins[0], mask & B1000);
         digitalWrite(leftPins[1], mask & B0100);
         digitalWrite(leftPins[2], mask & B0010);
         digitalWrite(leftPins[3], mask & B0001); 
         lastStepChange = micros();
       }
       rightFraction += rightPerStep;
       if(rightFraction >= 1.0) { //step right
         rightFraction -= 1.0;        
         if(rightSteps > 0) {
           rightSteps--;
           currRightPos++;
         }
         else if(rightSteps < 0) {
           rightSteps++;
           currRightPos--;
         }
         byte mask = stepSequence[currRightPos & 0x7];
         digitalWrite(rightPins[0], mask & B1000);
         digitalWrite(rightPins[1], mask & B0100);
         digitalWrite(rightPins[2], mask & B0010);
         digitalWrite(rightPins[3], mask & B0001); 
         lastStepChange = micros();
       } 
     
      if(accStop > 0 && startBreakingAt <= 0) {    
        delayMicros = min(delayMicros+MAX_ACCEL, MAX_DELAY);
      }      
      else if(accStart > 0) {
        delayMicros = max(delayMicros-MAX_ACCEL, MIN_DELAY);
      }  
      startBreakingAt--;
      
       delayMicroseconds(delayMicros); 
     }
   }  
   if((micros()-lastStepChange) > DISABLE_TIMEOUT) {     
     //disable steppers
     for(int pin=0 ; pin<4 ; pin++) {
       digitalWrite(leftPins[pin], 0); 
       digitalWrite(rightPins[pin], 0); 
     }
   }
}

void getCurrentPosition(int* left, int* right)
{
  *left = currLeftPos; 
  *right = currRightPos; 
}

