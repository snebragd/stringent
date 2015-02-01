#include <IRremote.h>

int RECV_PIN = 1;
IRrecv irrecv(RECV_PIN);
decode_results results;  

void setupIR() 
{
  irrecv.enableIRIn(); // Start the receiver  
}

void readIR()
{
  if (irrecv.decode(&results)) {
//    Serial.println(results.value, HEX);

    switch(results.value) {
       case 0xF50A4FB0:  //up
         manualPenUp = 1;
         break;
       case 0xF50ACF30:  //down
         manualPenDown = 1;
         break;
       case 0xf50a1de2: //left -
         manualLeft = -speedMult;
         break;
       case 0xf50aed12: //left +
         manualLeft = speedMult;
         break;
       case 0xc53ad926: //right -
         manualRight = -speedMult;
         break;
       case 0xc53a59a6: //right +
         manualRight = speedMult;
         break;
       case 0xf50af708:  //enter
         speedMult = 15.0;
         break;
       case 0xc53a7986: //play - reset center
         currentLeftSteps = 1000*stepsPerMM;
         currentRightSteps = 1000*stepsPerMM;       
         speedMult = 1.0;
         centerX = 500; //starting x pos
         centerY = 866; //starting x pos
         break;
       case 0xf50a857a:  //1
         printSize = 1; //print full size
         program = 1; //start print
         currentPlot = 0;
         break;
       case 0xf50a45ba:  //2
         printSize = 0.5;
         program = 1; //start print
         currentPlot = 1;
         break;
       case 0xf50ac53a:  //3
         printSize = 1;
         program = 1; //start print
         currentPlot = 3;
         break;
       case 0xF50A25DA:  //4
         printSize = 0.5;
         program = 1; //start print
         currentPlot = 2;
         break;
       case 0xF50AA55A:  //5
         printSize = 0.25;
         program = 1; //start print
         currentPlot = 2;
         break;
       case 0xF50A659A:  //6
         printSize = 0.125;
         program = 1; //start print
         currentPlot = 2;
         break;
    }   
    irrecv.resume(); // Receive the next value
  }  
}

