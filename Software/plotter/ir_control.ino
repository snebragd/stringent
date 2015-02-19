#include <IRremote.h>

#define RECV_PIN  1
IRrecv irrecv(RECV_PIN);
decode_results results;  

void setupIR() 
{
  irrecv.enableIRIn(); // Start the receiver  
}

void readIR()
{
  float lDist;
  
  if (irrecv.decode(&results)) {
//      Serial.println(results.value, HEX);
  
    switch(results.value) {
       case 0xF50A3DC2:  //power
         storePositionInEEPROM();         
         break;
       case 0xF50A4FB0:  //up
         printSize = 2;
         break;
       case 0xF50ACF30:  //down
         printSize = 0.5;
         break;
       case 0xf50a1de2: //left -
         manualLeft = -1;
         break;
       case 0xf50aed12: //left +
         manualLeft = 1;
         break;
       case 0xc53ad926: //right -
         manualRight = -1;
         break;
       case 0xc53a59a6: //right +
         manualRight = 1;
         break;
       case 0xC53AB946:  //prev - calibrate 200 mm from left
         currentLeftSteps = 200*stepsPerMM;         
         break;
       case 0xC53A39C6:  //next - calibrate 200 mm from right
         currentRightSteps = 200*stepsPerMM;              

         lDist = currentLeftSteps/stepsPerMM;
         disparity = (long)sqrt(lDist*lDist+200*200);
         break;
       case 0xf50af708:  //enter
         continousManualDrive = true;
         break;
       case 0xf50a2df0:  //return
         continousManualDrive = false;
         break;
       case 0xC53A19E6:  //stop
         stopPressed = true;
         break;
       case 0xc53a7986: //play - reset center for 1m-1m-1m triangle setup
//         currentLeftSteps = 1000*stepsPerMM;
//         currentRightSteps = 1000*stepsPerMM;     
//         disparity = 1000;  
         continousManualDrive = false;
//         centerX = 500; //starting x pos
//         centerY = 866; //starting x pos
         break;
      case 0xf50a857a:  //1
         program = 1; //start print
         currentPlot = 1;
         break;
       case 0xf50a45ba:  //2
         program = 1; //start print
         currentPlot = 2;
         break;
       case 0xf50ac53a:  //3
         program = 1; //start print
         currentPlot = 3;
         break;
       case 0xF50A25DA:  //4
         program = 1; //start print
         currentPlot = 4;
         break;
       case 0xF50AA55A:  //5
         program = 1; //start print
         currentPlot = 5;
         break;
       case 0xF50A659A:  //6
         program = 1; //start print
         currentPlot = 6;
         break;         
    }   
    irrecv.resume(); // Receive the next value
  }  
}

