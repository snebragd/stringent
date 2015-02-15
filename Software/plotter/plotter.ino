
//circumference of spool
//#define spoolCirc 127.5 //old larger spool 
#define spoolCirc 94.2 

//steps per full rotation
#define stepsPerRotation 4075.7728395

//number of steps for each full rotation
#define stepsPerMM (stepsPerRotation/spoolCirc)

//longest allowed line segment before splitting
#define maxSegmentLength 2 

//using serial debug will interfere with IR and Servo that are using pin 0 and 1 (TX/RX)
//#define SERIAL_DEBUG

#define EEPROM_LEFT_ADDR 0
#define EEPROM_RIGHT_ADDR 4
#define EEPROM_DISPARITY_ADDR 8

int state=0;
int currentPlot = 0;

long disparity = 1000;  //distance between anchor points 
long currentLeftSteps  = 1000*stepsPerMM; 
long currentRightSteps = 1000*stepsPerMM;
float centerX = 500; //starting x pos
float centerY = 866; //starting x pos

bool stopPressed = false;

int program = 0; //0 is start program, responding to IR 

//manual control
int manualLeft = 0, manualRight = 0, manualPenDown = 0, manualPenUp = 0;
float printSize = 1.0;
bool continousManualDrive = false;

//book keeping for sub segmenting lines
static float prevX = 0;
static float prevY = 0;
static int currentSubSegment = 0;

void setup()
{
#ifdef SERIAL_DEBUG
  //Initialize serial and wait for port to open:
    Serial.begin(9600); 
    Serial.println("Yo! debug at your service!");
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
  setOrigo();
  
  
#ifdef SERIAL_DEBUG
   //fake start a print since we dont have IR control
   printSize = 1;
   program = 1; //start print
   currentPlot = 1; 
#endif
  
}

void storePositionInEEPROM() {
  eepromWriteLong(EEPROM_LEFT_ADDR, currentLeftSteps);  
  eepromWriteLong(EEPROM_RIGHT_ADDR, currentRightSteps);  
  eepromWriteLong(EEPROM_DISPARITY_ADDR, disparity);  
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

void loop()
{        
    float tmpX, tmpY;
    int tmpPen;
    
    readIR(); 

    if(program == 0) {
      float left = (manualLeft/spoolCirc) * 360.0;    
      float right = (manualRight/spoolCirc) * 360.0;    
      currentLeftSteps += manualLeft*stepsPerMM;
      currentRightSteps += manualRight*stepsPerMM;
     
      step(manualLeft*stepsPerMM,manualRight*stepsPerMM, 0, 0);
      setOrigo();             

      if(stopPressed || (!continousManualDrive)) {             
        manualLeft = manualRight = 0;
        stopPressed = false;
      }
      
      if(manualPenDown) {
         movePen(true);
         manualPenDown = 0;
      }
      if(manualPenUp) {
         movePen(false);
         manualPenUp = 0;
      }            
    }
    else { 
      if(!getData(currentPlot, state, &tmpX, &tmpY, &tmpPen) || stopPressed) {
        //reached the end, go back to manual mode
        state = 0;
        program = 0;        
        
        //store current position in eeprom 
        storePositionInEEPROM();
      }
      else {      
        float nextX = tmpX*printSize;
        float nextY = tmpY*printSize;
        boolean nextPen = tmpPen > 0;

        float aStart = 1.0;
        float aStop = 1.0;

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
                  aStart = 0; //keep speed from previous segment
              }
              else {
                  advancePoint = false; //stay on same point        
                  currentSubSegment++;                  
                  
                  nextX = prevX + dx*currentSubSegment/subSegments;
                  nextY = prevY + dy*currentSubSegment/subSegments;
                  
                  if(currentSubSegment == 1) {
                    aStop = 0; //don't break
                  }
                  else if(currentSubSegment < subSegments) {
                    aStart = 0;
                    aStop = 0;
                  }
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
#ifndef SERIAL_DEBUG
          movePen(nextPen); //adjust pen as necessary  
          step(dLeft, dRight, aStart, aStop); //move steppers
#endif          
        }
      }      
  } 
}
