#include "MachineDefs.h"
 
//circumference of spool
#define spoolCirc 94.2 

//steps per full rotation (number from https://github.com/robjampar/Stepper , never bothered to calculate it propery myself)
#define stepsPerRotation 4075.7728395

//number of steps for each full rotation
#define stepsPerMM (stepsPerRotation/spoolCirc)

//longest allowed line segment before splitting
#define maxSegmentLength 10 

#define EEPROM_LEFT_ADDR 0
#define EEPROM_RIGHT_ADDR 4
#define EEPROM_DISPARITY_ADDR 8
#define EEPROM_STOPPED_AT_ADDR 12
#define EEPROM_CURRENT_PLOT_ADDR 16
#define EEPROM_PRINT_SIZE_ADDR 20
#define EEPROM_CENTER_X_ADDR 24
#define EEPROM_CENTER_Y_ADDR 28

long state=0;
long stoppedAt=0; //save of where we stopped, to allow resuming
int currentPlot = 0;
bool resumePlot = false;

//these numbers are left from early versions where setup was done by having exactly 1m of string reeled out on both left and right and a fix disparity of 1m between anchors. This is now set up by user instead.
long disparity = 1000;  //distance between anchor points 
long currentLeftSteps  = 1000*stepsPerMM; 
long currentRightSteps = 1000*stepsPerMM;
float centerX = 500; //starting x pos
float centerY = 866; //starting x pos

bool stopPressed = false;

int program = 0; //0 is start program, responding to IR 

//manual control
int manualLeft = 0, manualRight = 0;
float printSize = 1.0;
bool continousManualDrive = true;

//bookkeeping for sub segmenting lines
static float prevX = 0;
static float prevY = 0;
static int currentSubSegment = 0;

void storePositionInEEPROM();

void setup()
{  
#ifdef USE_DATA_FROM_DISK
  //store constant values into mock eeprom, wait 1sec first to allow storage
  delayMicroseconds(1000000);  
  storePositionInEEPROM();
  printf("[disparity, currentLeftSteps, currentRightSteps, centerX, centerY]\n");
  printf("%ld, %ld, %ld, %f, %f\n", disparity, currentLeftSteps, currentRightSteps, centerX, centerY);  
#endif
  
#ifdef SERIAL_DEBUG
  //Initialize serial and wait for port to open:
    Serial.begin(9600); 
    SER_PRINTLN("Setup");
#endif
  
  //initialize IR  
  setupIR();

  //initialize steppers
  setupStep();

  //initialize servo
  setupServo();
  
  //initialize SD card
  setupData();
  
  //read stored position from EEPROM
  currentLeftSteps = eepromReadLong(EEPROM_LEFT_ADDR);  
  currentRightSteps = eepromReadLong(EEPROM_RIGHT_ADDR);  
  disparity = eepromReadLong(EEPROM_DISPARITY_ADDR);  
  stoppedAt = eepromReadLong(EEPROM_STOPPED_AT_ADDR);  
  currentPlot = eepromReadLong(EEPROM_CURRENT_PLOT_ADDR);    
  if(stoppedAt > 0) {
    //preserve chosen size iff we stopped in the middle of a plot
    printSize = eepromReadFloat(EEPROM_PRINT_SIZE_ADDR);        
  }
  centerX = eepromReadFloat(EEPROM_CENTER_X_ADDR);    
  centerY = eepromReadFloat(EEPROM_CENTER_Y_ADDR);    
  
#ifdef SERIAL_DEBUG
  Serial.print("Disparity=");
  Serial.println(disparity);
#endif

#ifdef NO_REMOTE
   //fake start a print since we dont have IR control
   printSize = 1;
   program = 1; //start print
   currentPlot = 4; 
#endif  
}

unsigned long lastEEPromStore = 0;

void storePositionInEEPROM() {
  unsigned long now = micros();
  //dissallow storing to eeprom more than once a second (avoid repeated ir keystrokes)
  if((lastEEPromStore+1000000) <= now) {

    SER_PRINTLN("EEPROM: Store");

    lastEEPromStore = now;
    eepromWriteLong(EEPROM_LEFT_ADDR, currentLeftSteps);  
    eepromWriteLong(EEPROM_RIGHT_ADDR, currentRightSteps);  
    eepromWriteLong(EEPROM_DISPARITY_ADDR, disparity);  
    eepromWriteLong(EEPROM_STOPPED_AT_ADDR, stoppedAt);  
    eepromWriteLong(EEPROM_CURRENT_PLOT_ADDR, currentPlot);  
    eepromWriteFloat(EEPROM_PRINT_SIZE_ADDR, printSize);  
    eepromWriteFloat(EEPROM_CENTER_X_ADDR, centerX);  
    eepromWriteFloat(EEPROM_CENTER_Y_ADDR, centerY);  
  }
}

void setOrigo() {
    float currentLeft  = currentLeftSteps / stepsPerMM;
    float currentRight = currentRightSteps / stepsPerMM;
    float tmp1 = (currentRight*currentRight-disparity*disparity-currentLeft*currentLeft);
    float tmp2 = (-2*currentLeft*disparity);
    float a = acos(tmp1/tmp2);    
    centerX = currentLeft*cos(a);
    centerY = currentLeft*sin(a);
}

static int prevPen=0;

extern unsigned long lastBatteryLog;

void loop()
{        
    float tmpX, tmpY;
    int tmpPen;

#ifdef HAS_BATTERY_MEASUREMENT    
    unsigned long now = micros();
    if((lastBatteryLog+1000000) <= now) {
      lastBatteryLog = now;
      logBattery(now/1000000);
    }
#endif    
    readIR(); 

    if(program == 0) {
      float left = (manualLeft/spoolCirc) * 360.0;    
      float right = (manualRight/spoolCirc) * 360.0;    
      
      if(manualLeft != 0 || manualRight != 0) {
        currentLeftSteps += manualLeft*stepsPerMM;
        currentRightSteps += manualRight*stepsPerMM;
             
        step(manualLeft*stepsPerMM,manualRight*stepsPerMM,false);
        setOrigo();             
      }

      if(stopPressed || (!continousManualDrive)) {             
        manualLeft = manualRight = 0;
        stopPressed = false;
      }      
    }
    else { 
      if(!getData(currentPlot, state, &tmpX, &tmpY, &tmpPen) || stopPressed) {

        stoppedAt = stopPressed ? state : 0;
                
        //reached the end, go back to manual mode
        state = 0;
        program = 0;
        resumePlot = false; //make sure to not end up in loop if plot cannot be resumed due to missing file or corrupt data        

        //stop with pen up        
        movePen(false, false);
    
        step(0, 0, false); //flush out last line segment

        SER_PRINTLN("Plot done");
        
        //store current position in eeprom 
        storePositionInEEPROM();
      }
      else {
         if(resumePlot && stoppedAt > state) {
           //just skip points until we are at the point where we stopped
             state++;
             prevX = tmpX*printSize;
             prevY = tmpY*printSize;
         }
         else {        
          resumePlot = false;      
          float nextX = tmpX*printSize;
          float nextY = tmpY*printSize;
          boolean nextPen = tmpPen > 0;
          boolean advancePoint = true;
  
          if(state > 0) { //don't try to split first          
            float dx = nextX-prevX;
            float dy = nextY-prevY;
            float len = sqrt(dx*dx+dy*dy);
  
            if(len > maxSegmentLength) {
                //split segment
                int subSegments = 1 + (int)(len/maxSegmentLength);
                if(currentSubSegment == subSegments) {
                    //last segment
                    currentSubSegment = 0; //reset                                                                        
                }
                else {
                    advancePoint = false; //stay on same point        
                    currentSubSegment++;                  
                    
                    nextX = prevX + dx*currentSubSegment/subSegments;
                    nextY = prevY + dy*currentSubSegment/subSegments;
                }
            }
          }
  
          if(advancePoint) {
            state = state+1; //next point
            prevX = nextX;
            prevY = nextY;                   
          }
                  
          float xL = nextX+centerX;
          float xR = nextX+centerX-disparity;
          float y = nextY+centerY;
  
          long newLeft  = sqrt(xL*xL + y*y)*stepsPerMM;
          long newRight = sqrt(xR*xR + y*y)*stepsPerMM;
          
          long dLeft  = (newLeft  - currentLeftSteps);            
          long dRight = (newRight - currentRightSteps);
                  
          currentLeftSteps = newLeft;
          currentRightSteps = newRight;
    
          if(((dLeft == 0) && (dRight == 0))) {
            //no move, ignore
          }
          else {
            movePen(prevPen, false); //adjust pen as necessary  
            step(dLeft, dRight, prevPen != nextPen); //move steppers
            prevPen=nextPen;
          }
      }
    }      
  } 

  //check and disable steppers if idle
  checkDisableSteppers();
}
