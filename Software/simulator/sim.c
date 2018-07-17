#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <unistd.h>

typedef int bool;
typedef int boolean;
#define false 0
#define true 1

#define delayMicroseconds my_usleep

static int time_multiplier = 1;
static bool fake_sleep = false;
static unsigned long fake_time=0;

int my_usleep(unsigned long usec)
{
  fake_time += usec;
  if(!fake_sleep) {
    usleep(usec/time_multiplier);
  }
  return 0;
}

unsigned long micros()
{
  //  if(fake_sleep) {
    return fake_time;
    //  }
    //else {
    //struct timeval tv;
    //gettimeofday(&tv,NULL);
    ////    printf("micros=%ld\n", tv.tv_usec);
    //return time_multiplier*(tv.tv_sec*1000000 + tv.tv_usec);
    //}
}

//************** Mock servo *************
//disable to preserve space
#define USE_SMOOTH_SERVO
//#define USE_LINEAR_SERVO

#define UP 100
#define DOWN 20

static int oldPos = 0;

void movePen(boolean down, boolean fast)
{
#ifndef SERIAL_DEBUG  
  int pos = down ? DOWN : UP;
  if(pos != oldPos) {

#ifdef USE_SMOOTH_SERVO    
    if(fast) {
	printf("%ld %d=%d\n",micros(), 0,(int)pos);        
    }
    else {
      for(float i=0 ; i<=1.0 ; i+=0.01) {
	float i2 = (1-cos(i*3.14))/2;
	float tmpPos = (oldPos*(1.0-i2)+(pos*i2));

	//     myservo.write((int)tmpPos);
	printf("%ld %d=%d\n",micros(), 0,(int)tmpPos);
	
	delayMicroseconds(10000); 
      }      
    }      
#else //USE_SMOOTH_SERVO
    //  myservo.write((int)pos);
    printf("%ld %d=%d\n",micros(), 0,(int)pos);
#endif //USE_SMOOTH_SERVO

    oldPos = pos;
  }
#endif
}

void setupServo() 
{
  movePen(false, true);
}

void makePenNoise()
{
  int p = oldPos;  
  for(int i=0;i<4;i++) {
    //    myservo.write((int)p-15);    
    delayMicroseconds(400000); 
    //    myservo.write((int)p);    
    delayMicroseconds(400000); 
  }  
}


//*********************** mock eeprom ******************
long eepromdata_long[100];
float eepromdata_float[100];

void eepromWriteLong(int addr, long data) {
  eepromdata_long[addr] = data;
  //  printf("eeprom[%d]=%ld", addr, data);
}

void eepromWriteFloat(int addr, float data) {
  eepromdata_float[addr] = data;
  //  printf("eeprom[%d]=%f", addr, data);
}

long eepromReadLong(int addr) {
  long ret =  eepromdata_long[addr];
  // printf("eeprom[%d] -> %ld", addr, ret);
  return ret;
}

float eepromReadFloat(int addr) {
  float ret = eepromdata_float[addr];
  //printf("eeprom[%d] -> %f", addr, ret);
  return ret;
}
//********************** mock IR ****************************
FILE* ir_file = NULL;
void setupIR() 
{
   ir_file = fopen("irdata.txt", "r+");
}

extern int currentPlot;
extern int program;

void readIR()
{
  //just read from file what plot to start and kick it off
  if(program == 0) {
    if(ir_file) {
      char buf[100];
      if(fgets(buf, 99, ir_file)) {
	program = 1; //start print
	currentPlot = atoi(buf);
	//	fprintf(stderr,"%d\n",currentPlot);
      }
      else {
	fprintf(stderr, "plot time = %.1f s\n", micros()/1000000.0);
	exit(0);
      }
    }
  }
}


//****************** include arduino code *********************
#define USE_DATA_FROM_DISK
#define USE_MOCKED_STEPPERS

//inlude data ino file
#define File FILE*
#define PROGMEM
#define prog_int16_t short
#define OUTPUT 0
void pinMode(int p, int m) {}
#define FILE_READ "r"
#define pgm_read_word_near *
#define tmp_seek(a,b) fseek(a,b,SEEK_SET)
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)>(b)?(b):(a))
extern long disparity;
#include "/tmp/data.ino"

//include stepper file
typedef unsigned char byte;
#define PI 3.1415926535
#define B1000 0b1000
#define B1100 0b1100
#define B0100 0b0100
#define B0110 0b0110
#define B0010 0b0010
#define B0011 0b0011
#define B0001 0b0001
#define B1001 0b1001
#define digitalWrite(pin, val) printf("%lu %d=%d\n",micros(), (pin),(val))
#define spoolCirc 94.2 
#define stepsPerRotation 4075.7728395
#define stepsPerMM (stepsPerRotation/spoolCirc)
#define abs(X) abs((int)(X))

#include "../plotter/step_control.ino"

//inlude main plotter file
#include "../plotter/plotter.ino"


int main(int argc, char* argv[])
{  
  if(argc > 1) {
    time_multiplier = atoi(argv[1]);
    if(time_multiplier == 0) {
      time_multiplier = 1;
    }
    else if(time_multiplier > 100) {
      fake_sleep = true; //OS sleep functions will just wait too long for really short naps, so lets fake it all.
    }
  }

  setup();
  for(;;) {
    loop();
  }

  return 0;
}
