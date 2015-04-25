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

#define delayMicroseconds usleep

unsigned long micros() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  //    printf("micros=%ld\n", tv.tv_usec);
  return tv.tv_sec*1000000 + tv.tv_usec;
}

//************** Mock servo *************
//disable to preserve space
#define USE_SMOOTH_SERVO
//#define USE_LINEAR_SERVO

#define UP 100
#define DOWN 20

static int oldPos = 0;

void movePen(boolean down)
{
#ifndef SERIAL_DEBUG  
  int pos = down ? DOWN : UP;
  if(pos != oldPos) {

#ifdef USE_SMOOTH_SERVO    

#ifdef USE_LINEAR_SERVO    
    while(pos != oldPos) {
      //            myservo.write(oldPos);
      oldPos = oldPos + ((pos-oldPos) > 0 ? 1 : -1);
       delayMicroseconds(5000); 
    }
#else //USE_LINEAR_SERVO    
    for(float i=0 ; i<=1.0 ; i+=0.01) {
      float i2 = (1-cos(i*3.14))/2;
      float tmpPos = (oldPos*(1.0-i2)+(pos*i2));
      //     myservo.write((int)tmpPos);
      delayMicroseconds(10000); 
    }
#endif //USE_LINEAR_SERVO

    oldPos = pos;
#else //USE_SMOOTH_SERVO
    //  myservo.write((int)pos);
    oldPos = pos;
#endif //USE_SMOOTH_SERVO
  }
#endif
}

void setupServo() 
{
  movePen(false);  
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
  printf("eeprom[%d]=%ld", addr, data);
}

void eepromWriteFloat(int addr, float data) {
  eepromdata_float[addr] = data;
  printf("eeprom[%d]=%f", addr, data);
}

long eepromReadLong(int addr) {
  long ret =  eepromdata_long[addr];
  printf("eeprom[%d] -> %ld", addr, ret);
  return ret;
}

float eepromReadFloat(int addr) {
  float ret = eepromdata_float[addr];
  printf("eeprom[%d] -> %f", addr, ret);
  return ret;
}
//********************** mock IR ****************************
void setupIR() 
{
}

extern int currentPlot;
extern int program;

void readIR()
{
  FILE* f = fopen("/tmp/irdata.txt", "r+");
  if(f) {
    program = 1; //start print
    currentPlot = 1;

    fclose(f);
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
#define digitalWrite(pin, val) printf("pin %d=%d ",(pin),(val))
#define spoolCirc 94.2 
#define stepsPerRotation 4075.7728395
#define stepsPerMM (stepsPerRotation/spoolCirc)
#include "../plotter/step_control.ino"

//inlude main plotter file
#include "../plotter/plotter.ino"


int main(int argc, char* argv[])
{  
  setup();
  for(;;) {
    loop();
  }

  return 0;
}
