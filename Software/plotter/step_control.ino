
//current stepper position in step sequence (not global position)
long currLeftPos = 0;
long currRightPos = 0;

byte stepSequence[8] = {B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001};

//original order
//byte leftPins[4] = {6,7,8,9};
//byte rightPins[4] = {2,3,4,5};

//flipped order
byte leftPins[4] = {9,8,7,6};
byte rightPins[4] = {5,4,3,2};

//shortest reliable delay seems to be around 600 (1220 used previously)
#define MIN_DELAY 600
#define MAX_DELAY 20000

//Allowed speed in steps/us
#define MAX_SPEED (1.0/MIN_DELAY)
#define MIN_SPEED (1.0/MAX_DELAY)

//Current speed in steps / ms
float currentSpeed = MIN_SPEED; 

//distance travelled before reaching full speed
//#define FULL_SPEED_DIST (maxSegmentLength/2.0)
#define FULL_SPEED_DIST 20.0

//acceleration in speed / step (yes, weird unit).
#define D_SPEED ((MAX_SPEED-MIN_SPEED) / (stepsPerMM*FULL_SPEED_DIST))

unsigned int delayMicros = 1.0/MAX_SPEED;

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
  accStop = false;
  
  long numSteps = max(abs(leftSteps), abs(rightSteps));

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
     
      numSteps = max(abs(leftSteps), abs(rightSteps));     
      if(accStop && currentSpeed >= (numSteps*D_SPEED + MIN_SPEED)) {    
        //start breaking
        currentSpeed = max(currentSpeed-(currentSpeed/numSteps), MIN_SPEED);
      }      
      else {
        currentSpeed = min(currentSpeed+D_SPEED, MAX_SPEED);
      }  
      
       delayMicroseconds(1.0 / currentSpeed); 
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

