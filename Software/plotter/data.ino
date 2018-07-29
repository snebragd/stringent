#include "MachineDefs.h"
 
#ifndef USE_DATA_FROM_DISK

#include <avr/pgmspace.h>
#include <SPI.h>
#include <SD.h>

#endif

bool getDataInternal(int plotNo, int point, float *x, float* y, int* pen);
bool getSvgData(int plotNo, int point, float *x, float* y, int* pen);
bool getMemoryData(int plotNo, int point, int *x, int* y, int* pen);

static int currentlySelectedPlot = -1;

static File svgFile;

static int reachedTheEndAt = -10;

bool getData(int plotNo, int point, float *x, float* y, int* pen)
{
  if(point == (reachedTheEndAt+1)) {
    reachedTheEndAt = -10; 
    return false;
  }
  else {
    if(!getDataInternal(plotNo, point, x, y, pen)) {
      //return to origo on end
      *x = *y = 0.0;
      *pen = 0;
      reachedTheEndAt = point; 
    }    
    return true;
  }
}

bool getDataInternal(int plotNo, int point, float *x, float* y, int* pen)
{  
  if(currentlySelectedPlot != plotNo) {
    //first call, check if we have plot stored on SD
    
    if(svgFile) {
      svgFile.close();
      svgFile = SD.open("dummy_fail_this_open", FILE_READ);;      
    }
    
    char* svgName = (char*)"1.svg";
    svgName[0] = '0'+plotNo;    
    svgFile = SD.open(svgName, FILE_READ);
    if(svgFile) {
        //found svg
      SER_PRINT("Found: ");      
      SER_PRINTLN(svgName);      
    }
    else {
      makePenNoise(3);
      SER_PRINT("No such file: ");      
      SER_PRINTLN(svgName);      
      return false;
    }

    currentlySelectedPlot = plotNo;
  }
  
  if(svgFile) {
    return getSvgData(plotNo, point, x, y, pen);    
  }
  else
  {
    return false;
  }
}

#ifdef HAS_BATTERY_MEASUREMENT

static File batteryFile;
unsigned long lastBatteryLog = 0;

static float batteryAverage=800; //just start with something above threshold
#define BATTERY_THRESHOLD 650

void logBattery(int secsSinceStart) {
#ifndef USE_DATA_FROM_DISK  
  int batt = analogRead(0);
  batteryFile.print(secsSinceStart);
  batteryFile.print(' ');
  batteryFile.println(batt);
  batteryFile.flush();
  
  batteryAverage = 0.99*batteryAverage + 0.01*batt;
  if(batteryAverage < BATTERY_THRESHOLD) {
    batteryFile.println("LOW BATTERY!");
    batteryFile.flush();
    stopPressed = true; //stop plot and persist state to eeprom to allow resume after battery has been changed
  }  
#endif
}

#endif //HAS_BATTERY_MEASUREMENT

void setupData()
{
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    SER_PRINTLN("SD fail");
    // don't do anything more:
    return;
  }  
#ifdef HAS_BATTERY_MEASUREMENT  
#ifndef USE_DATA_FROM_DISK
  batteryFile = SD.open("battery.log", FILE_WRITE);;      
#endif
#endif //HAS_BATTERY_MEASUREMENT
}

// **************** Svg path ***************************
static int lastReadPoint = -1;

bool
seekTo(char* pattern)
{
  //yes, this is flawed string matching
  char* tmp = pattern;

  while(*tmp != '\0') {
      char c = svgFile.read();
      if(c < 0) {
         return false;
      }
      if(c == *tmp) {
        tmp++;
      }
      else {
        tmp = pattern;        
      }      
  }
  return true;  
}

bool
seekToPathStart(bool rewindFile) {
  if(rewindFile) {
    svgFile.seek(0);  //rewind (we could have paused)
  }
    if(!seekTo((char*)"<path")) {
//      SER_PRINTLN("No <path> tag");    
      return false;
    }
    
    if(!seekTo((char*)" d=\"")) {
      SER_PRINTLN("No d=\" in path");    
      return false;
    }
  return true;  
}

//terribly sorry about this, but didn't have program space for using atof
bool
readFloat(float* ret) {
  char tmp[20];
  float f=0;
  bool pastPoint=false;
  bool exp=0;
  float div = 1;
  
  for(int i=0 ; i<20 ; i++) {
    tmp[i] = svgFile.read();
    if(tmp[i]<0) {
      return false;
    } 
    else if((tmp[i] >= '0') && (tmp[i] <= '9')) {
      if(exp > 0) {
	for (int e=(tmp[i]-'0') ; e > 0; e--) {
	  div = div/10;
	}
      }
      if(exp <  0) {
	for (int e=(tmp[i]-'0') ; e > 0; e--) {
	  div = div*10;
	}
      }
      else if(div < 100) { 
        f = f*10+(tmp[i]-'0');      
        if(pastPoint) {
           div = div*10;
        }
      }
      else {
        //only care for two decimals
      }
    }
    else if(tmp[i] == '.') {
      pastPoint=true;
    }
    else if(tmp[i] == '-') {
      if(exp != 0) {
	exp = -1;
      }
      else {
	div = -1;
      }
    }
    else if(tmp[i] == 'e') {
      exp = 1;
    }
    else {
      break;
    }   
  }  
  *ret = f/div;
  
  return true;
}

bool
getNextPathSegment(float *x, float *y, int *line, bool first)
{
  char c;
  bool rel = false;
  static float lastX, lastY;

  while(true) {
    if(first) {
      lastX=lastY=0.0;
    }

    c = svgFile.read();
    //    fputc(c,stdout);
    if(c == 'M') {
      *line = 0;     
    }
    else if(c=='m') {
      *line = 0;     
      svgFile.read(); //eat a space byte
      rel = true;
    }
    else if(c == 'L') {
      *line = 1;
    }
    else if(c==' ' || c=='l') {
      *line = 1;
      rel = true;
      continue;
    }
    else if(((c >= '0') && (c <= '9')) || c=='-') {
      svgFile.seek(svgFile.position()-1);       
    }
    else {
      //reached end, look for another <path
      if(seekToPathStart(false)) {
	first = true;
	continue;
      }
      else {
	return false;
      }
    }      
    break;
  }
  
  if(!readFloat(x)) {
    return false;    
  } 
  if(!readFloat(y)) {
    return false;    
  }

  if(rel) {
    *x += lastX;
    *y += lastY;
  }
  lastX = *x;
  lastY = *y;
 
  //rewind one byte that was eaten by last float read  
  svgFile.seek(svgFile.position()-1); 
    
  return true; 
}

static float min_x = 100000000.0;
static float min_y = 100000000.0;
static float scaleFactor = 1.0;

static float lastX,lastY;
static int lastPen;

bool getSvgData(int plotNo, int point, float *x, float* y, int* pen)
{
  if(point == 0) {
    long pathPosition;
    long segments = 0;
    float max_x, max_y;
    
    lastReadPoint = -1;
    
    //first read, get dimensions
    if(!seekToPathStart(true)) {      
      SER_PRINTLN("No path found!");    
      return false;
    }    
    SER_PRINTLN("Found <path>!");    
    pathPosition = svgFile.position();

    min_y = min_x = 100000000.0;
    max_y = max_x = -100000000.0;

    while(true) {
      if(getNextPathSegment(x, y, pen,segments==0)) {
        segments++;        
        min_x = min(min_x, *x);
        max_x = max(max_x, *x);
        min_y = min(min_y, *y);
        max_y = max(max_y, *y);
      }
      else {
        break;
      }
    }
    scaleFactor = (disparity*0.4) / (max_x-min_x); //fill 40% of disparity as default
    
    SER_PRINT("Segments=");    
    SER_PRINTLN(segments);    

    SER_PRINT("Scale=");    
    SER_PRINTLN(scaleFactor);    

#ifdef USE_MOCKED_STEPPERS
      fprintf(stderr,"segments=%3ld scale=%2.2f x=[%2.2f , %2.2f] y=[%2.2f , %2.2f] disparity=%ld\n", segments, scaleFactor, min_x,max_x, min_y, max_y, disparity);
#endif        

    svgFile.seek(pathPosition);    
  }

  if(point != lastReadPoint) {

    lastReadPoint = point;

    if(getNextPathSegment(x, y, pen,point==0)) {
      *x = (*x-min_x)*scaleFactor;   
      *y = (*y-min_y)*scaleFactor;
   
      lastX = *x;
      lastY = *y;
      lastPen = *pen;   
    }
    else {
      lastX = 0;
      lastY = 0;
      lastPen = 0;
      // rewind the file:
      svgFile.seek(0);    
      
      return false;
    }    
  }
  else {
    *x = lastX;    
    *y = lastY;    
    *pen = lastPen;    
  }

  SER_PRINT(*pen ? "L " : "M ");    

  return true;
}

