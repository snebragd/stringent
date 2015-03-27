
long currLeftPos = 0;
long currRightPos = 0;

byte stepSequence[8] = {B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001};

//original order
//byte leftPins[4] = {6,7,8,9};
//byte rightPins[4] = {2,3,4,5};

//flipped order
byte leftPins[4] = {9,8,7,6};
byte rightPins[4] = {5,4,3,2};

//#define MIN_DELAY 600
#define MIN_DELAY 1220
#define MAX_DELAY 20000
#define MAX_ACCEL ((MAX_DELAY-MIN_DELAY)/((maxSegmentLength/2)*stepsPerMM))

unsigned int delayMicros = MIN_DELAY;

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

static float prevDir;
static long leftSteps=0;
static long rightSteps=0;
static bool accStop=true; //did we break the last segment

#define ACC_DIR_THRESHOLD (PI/4)
#define DIR_CHANGE_WAIT 1000
/*
void stepWithFraction(float *fraction, long *steps, long *currPos, byte *pins)
{
  if(*fraction >= 1.0) { //step
     *fraction -= 1.0;        

     if(*steps > 0) {
       *steps--;
       *currPos++;
     }
     else if(*steps < 0) {
       *steps++;
       *currPos--;
     }
     byte mask = stepSequence[*currPos & 0x7];

     for(int bit=0 ; bit<4 ; bit++) {        
       digitalWrite(pins[bit], mask & (B1000 >> bit));
     }
     
     lastStepChange = micros();
   }
}
*/
void step(long nextLeftSteps, long nextRightSteps)
{
  bool accStart=accStop; //only accellerate if we breaked the previous segment
  accStop = false;
  
  long numSteps = max(abs(leftSteps), abs(rightSteps));
  long startBreakingAt = numSteps;

  float nextDir = atan2(nextLeftSteps,nextRightSteps);  
  float diff = abs(nextDir-prevDir);
  if(diff > PI) {
    diff = 2*PI-diff;
  }
  
  if(diff > ACC_DIR_THRESHOLD) {
    //sharp turn, break
    accStop = true;
//    for(int i=0; i<DIR_CHANGE_WAIT ; i++) {
//      delayMicroseconds(1000); 
//    }
  } 
  prevDir = nextDir;  
  
  delayMicros = (accStart > 0) ? MAX_DELAY : MIN_DELAY;

  if(accStop) {    
    startBreakingAt = numSteps-(MAX_DELAY-MIN_DELAY)/MAX_ACCEL;    
  }
  
  if(numSteps > 0) {
     float leftPerStep = abs(leftSteps) / (float)numSteps;
     float rightPerStep = abs(rightSteps) / (float)numSteps;     
     float leftFraction = 0;
     float rightFraction = 0;
    
     while(abs(leftSteps) > 0 || abs(rightSteps) > 0) {

       leftFraction += leftPerStep;
//       stepWithFraction(&leftFraction, &leftSteps, &currLeftPos, leftPins);

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

         for(int bit=0 ; bit<4 ; bit++) {        
           digitalWrite(leftPins[bit], mask & (B1000 >> bit));
         }
         
         lastStepChange = micros();
       } 
       rightFraction += rightPerStep;
//       stepWithFraction(&rightFraction, &rightSteps, &currRightPos, rightPins);
       
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
         
         for(int bit=0 ; bit<4 ; bit++) {        
           digitalWrite(rightPins[bit], mask & (B1000 >> bit));
         }
         
         lastStepChange = micros();
       }  
     
      if(accStop && startBreakingAt <= 0) {    
        delayMicros = min(delayMicros+MAX_ACCEL, MAX_DELAY);
      }      
      else if(accStart) {
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
   
   leftSteps = nextLeftSteps;
   rightSteps = nextRightSteps;
}

void getCurrentPosition(int* left, int* right)
{
  *left = currLeftPos; 
  *right = currRightPos; 
}

