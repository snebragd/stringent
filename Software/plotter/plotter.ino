
//circumference of spool
//#define spoolCirc 127.5 //old larger spool 
#define spoolCirc 94.2 

//steps per full rotation
#define stepsPerRotation 4075.7728395

//number of steps for each full rotation
#define stepsPerMM (stepsPerRotation/spoolCirc)

//longest allowed line segment before splitting
#define maxSegmentLength 2 

int state=0;
int currentPlot = 0;

float disparity = 1000;  //distance between anchor points 
long currentLeftSteps  = 1000*stepsPerMM; 
long currentRightSteps = 1000*stepsPerMM;
float centerX = 500; //starting x pos
float centerY = 866; //starting x pos

int program = 0; //0 is start program, responding to IR 

//manual control
int manualLeft = 0, manualRight = 0, manualPenDown = 0, manualPenUp = 0;
float printSize = 1.0;
int speedMult = 15;

//book keeping for sub segmenting lines
static float prevX = 0;
static float prevY = 0;
static int currentSubSegment = 0;

void setup()
{
  //Initialize serial and wait for port to open:
//  Serial.begin(9600); 
//  Serial.println("Yo! debug at your service!"); 

  //initialize IR  
  setupIR();

  //initialize steppers
  setupStep();

  //initialize servo
  setupServo();
  
  //initialize SD card
  setupData();
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
    int tmpX, tmpY, tmpPen;
    
    readIR(); 

    if(program == 0) {
      float left = (manualLeft/spoolCirc) * 360.0;    
      float right = (manualRight/spoolCirc) * 360.0;    
      currentLeftSteps += manualLeft*stepsPerMM;
      currentRightSteps += manualRight*stepsPerMM;
     
      step(manualLeft*stepsPerMM,manualRight*stepsPerMM, 0, 0);
      setOrigo();             
            
      manualLeft = manualRight = 0;

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
      if(!getData(currentPlot, state, &tmpX, &tmpY, &tmpPen)) {
        //reached the end, go back to manual mode
        state = 0;
        program = 0;        
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
        
        //start by moving pen before setting up steppers
        movePen(nextPen);
        
        float xL = nextX+centerX;
        float xR = nextX+centerX-disparity;
        float y = nextY+centerY;

        long newLeft  = sqrt(xL*xL + y*y)*stepsPerMM;
        long newRight = sqrt(xR*xR + y*y)*stepsPerMM;
        
        long dLeft  = (newLeft  - currentLeftSteps);            
        long dRight = (newRight - currentRightSteps);
                
        currentLeftSteps = newLeft;
        currentRightSteps = newRight;

        step(dLeft, dRight, aStart, aStop);

/*        float newLeft = sqrt(xL*xL + y*y);  
        float newRight = sqrt(xR*xR + y*y);  

        float dLeft = (newLeft-currentLeft);            
        float dRight = (newRight-currentRight);
             
        currentLeft = newLeft;
        currentRight = newRight;
                
        step(dLeft*stepsPerMM, dRight*stepsPerMM, aStart, aStop);*/
      }      
  } 
}
