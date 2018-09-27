//#include <IRremote.h>
#include "MachineDefs.h"

#include <ESP8266WiFi.h>

const char* ssid = "your-wifi-ssid";
const char* password = "your-wifi-password";

WiFiServer server(80);

 
//IRrecv irrecv(RECV_PIN);
//decode_results results;  

#ifdef PIONEER_DVD_REMOTE
/******************** PIONEER_DVD_REMOTE **********************/
#define CODE_BIG_PRINT 0xF50A4FB0    //up
#define CODE_SMALL_PRINT 0xF50ACF30  //downd
#define CODE_LEFT_REEL_IN 0xf50a1de2   //step
#define CODE_LEFT_REEL_OUT 0xf50aed12  //slow
#define CODE_RIGHT_REEL_IN 0xc53ad926   //rev
#define CODE_RIGHT_REEL_OUT 0xc53a59a6  //fwd
#define CODE_LEFT_CALIBRATION 0xC53AB946  //prev
#define CODE_RIGHT_CALIBRATION 0xC53A39C6 //next
#define CODE_ENABLE_CONT_DRIVE 0xf50a2df0  //return
#define CODE_DISABLE_CONT_DRIVE 0xf50af708 //enter
#define CODE_STOP 0xC53A19E6 //pause
#define CODE_RESUME 0xc53a7986 //play
#define CODE_1 0xf50a857a
#define CODE_2 0xf50a45ba
#define CODE_3 0xf50ac53a
#define CODE_4 0xF50A25DA
#define CODE_5 0xF50AA55A
#define CODE_6 0xF50A659A
#define CODE_7 0xF50AE51A
#define CODE_8 0xF50A15EA
#define CODE_9 0xF50A956A
#define CODE_0 0xF50A05FA

#elif defined NONAME_WHITE_REMOTE
/******************** NONAME_WHITE_REMOTE **********************/
#define CODE_BIG_PRINT 0xff02fd //+
#define CODE_SMALL_PRINT 0xff9867 //-
#define CODE_LEFT_REEL_IN 0xff22dd //test
#define CODE_LEFT_REEL_OUT 0xffe01f //left
#define CODE_RIGHT_REEL_IN 0xffc23d //back
#define CODE_RIGHT_REEL_OUT 0xff906f //right
#define CODE_LEFT_CALIBRATION 0xffa25d //power
#define CODE_RIGHT_CALIBRATION 0xffe21d //menu
#define CODE_ENABLE_CONT_DRIVE 0xffb04f //c
#define CODE_DISABLE_CONT_DRIVE 0xBADC0DE //not button for this shit 
#define CODE_STOP 0xff6897 //0
#define CODE_RESUME 0xffa857 //play
#define CODE_1 0xff30cf
#define CODE_2 0xff18e7
#define CODE_3 0xff7a85
#define CODE_4 0xff10ef
#define CODE_5 0xff38c7
#define CODE_6 0xff5aa5
#define CODE_7 0xff42bd
#define CODE_8 0xff4ab5
#define CODE_9 0xff52ad

#elif defined NODEMCU_WEBSERVER
/******************** NODEMCU_WEBSERVER **********************/
#define CODE_BIG_PRINT 1
#define CODE_SMALL_PRINT 2
#define CODE_LEFT_REEL_IN 3
#define CODE_LEFT_REEL_OUT 4
#define CODE_RIGHT_REEL_IN 5
#define CODE_RIGHT_REEL_OUT 6
#define CODE_LEFT_CALIBRATION 7
#define CODE_RIGHT_CALIBRATION 8
#define CODE_ENABLE_CONT_DRIVE 9
#define CODE_DISABLE_CONT_DRIVE 10
#define CODE_STOP 11 //0
#define CODE_RESUME 12 //play
#define CODE_1 13
#define CODE_2 14
#define CODE_3 15
#define CODE_4 16
#define CODE_5 17
#define CODE_6 18
#define CODE_7 19
#define CODE_8 20
#define CODE_9 21

#else
#error What remote?
#endif 


void setupIR()
{
  // irrecv.enableIRIn(); // Start the receiver
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}


void readIR()
{
  float lDist;
  bool fail = false;

  //if (irrecv.decode(&results)) {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  #ifdef SERIAL_DEBUG
  Serial.println("new client");
  #endif
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  #ifdef SERIAL_DEBUG
  Serial.println(request);
  #endif
  client.flush();

  // Match the request
  int valStartPos = request.indexOf("?a=b") + 4;
  int valEndPos = request.indexOf(" HTTP/1.1");
  int value = request.substring(valStartPos, valEndPos).toInt();

  switch (value) {
#ifndef NO_REMOTE
    case 0xF50A3DC2:  //power
      storePositionInEEPROM();

      break;
    case CODE_BIG_PRINT:  //up
      printSize = 2;
      Serial.print("Option Selected: ");
      Serial.println(value);
      break;
    case CODE_SMALL_PRINT:  //down
      printSize = 0.5;
      break;
    case CODE_LEFT_REEL_IN: //left -
      manualLeft = -1;
      break;
    case CODE_LEFT_REEL_OUT: //left +
      manualLeft = 1;
      break;
    case CODE_RIGHT_REEL_IN: //right -
      manualRight = -1;
      break;
    case CODE_RIGHT_REEL_OUT: //right +
      manualRight = 1;
      break;
    case CODE_LEFT_CALIBRATION:  //prev - calibrate 200 mm from left
      currentLeftSteps = 200 * stepsPerMM;
      //testPen();
      break;
    case CODE_RIGHT_CALIBRATION:  //next - calibrate 200 mm from right
      currentRightSteps = 200 * stepsPerMM;

      lDist = currentLeftSteps / stepsPerMM;
      disparity = (long)sqrt(lDist * lDist - 200L * 200L);

      SER_PRINT("Cal: lDist)");
      SER_PRINTLN(lDist);

      SER_PRINT("Cal: Disp=");
      SER_PRINTLN(disparity);

      break;
    case CODE_DISABLE_CONT_DRIVE:
      continousManualDrive = false;
      break;
    case CODE_ENABLE_CONT_DRIVE:
      continousManualDrive = true;
      break;
    case CODE_STOP:
      stopPressed = true;
#if  CODE_DISABLE_CONT_DRIVE == 0xBADC0DE
      //just disable continous drive when pressing stop. Re-enable with CODE_ENABLE_CONT_DRIVE again
      continousManualDrive = false;
#endif
      break;
    case CODE_RESUME:
      //resume print, or start new
      program = 1; //start print
      resumePlot = true;
      break;
    case CODE_1: program = 1; currentPlot = 1; break;
    case CODE_2: program = 1; currentPlot = 2; break;
    case CODE_3: program = 1; currentPlot = 3; break;
    case CODE_4: program = 1; currentPlot = 4; break;
    case CODE_5: program = 1; currentPlot = 5; break;
    case CODE_6: program = 1; currentPlot = 6; break;
    case CODE_7: program = 1; currentPlot = 7; break;
    case CODE_8: program = 1; currentPlot = 8; break;
    case CODE_9: program = 1; currentPlot = 9; break;
    default:
      //Serial.print("Option Selected: ");
      //Serial.println(value);
      fail = true;
#endif //NO_REMOTE
  }
  if (fail) {
    SER_PRINT("???: ");
  }
  else {
    makePenNoise(1);
  }
  SER_PRINTLN2(value, HEX);

  // irrecv.resume(); // Receive the next value

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html> \
  <head> \
    <title>Plotter Controls - Start/Stop One Stepper</title> \
    <style> \
      body{ \
        font-family:Helvetical, Arial, Sans-Serif; \
      } \
      a{ \
        border-radius:1em; \
        border:2px solid #269; \
        width:30%; \
        margin:2%; \
        padding:2em; \
        float:left; \
        text-decoration:none; \
        font-size:2em; \
        text-align:center; \
        color:white; \
        background-color:#369; \
        } \
         \
      a.chosen{ \
        color:#369; \
        background-color:white; \
        border:2px solid #369; \
      } \
    </style> \
  </head> \
  <body>");


  client.print("<a id='b1' href='?a=b1'");
  if (value == 1) {
    client.print("class='chosen'");
  }
  client.print(">Big Print</a>");



  client.print("<a id='b2' href='?a=b2'");
  if (value == 2) {
    client.print("class='chosen'");
  }
  client.print(">Small Print</a>");



  client.print("<a id='b3' href='?a=b3'");
  if (value == 3) {
    client.print("class='chosen'");
  }
  client.print(">Left Reel In</a>");



  client.print("<a id='b4' href='?a=b4'");
  if (value == 4) {
    client.print("class='chosen'");
  }
  client.print(">Left Reel Out</a>");



  client.print("<a id='b5' href='?a=b5'");
  if (value == 5) {
    client.print("class='chosen'");
  }
  client.print(">Right Reel In</a>");



  client.print("<a id='b6' href='?a=b6'");
  if (value == 6) {
    client.print("class='chosen'");
  }
  client.print(">Right Reel Out</a>");



  client.print("<a id='b7' href='?a=b7'");
  if (value == 7) {
    client.print("class='chosen'");
  }
  client.print(">Left Calibration</a>");



  client.print("<a id='b8' href='?a=b8'");
  if (value == 8) {
    client.print("class='chosen'");
  }
  client.print(">Right Calibration</a>");



  client.print("<a id='b9' href='?a=b9'");
  if (value == 9) {
    client.print("class='chosen'");
  }
  client.print(">Enable Cont Drive</a>");



  client.print("<a id='b10' href='?a=b10'");
  if (value == 10) {
    client.print("class='chosen'");
  }
  client.print(">Disable Cont Drive</a>");



  client.print("<a id='b11' href='?a=b11'");
  if (value == 11) {
    client.print("class='chosen'");
  }
  client.print(">Stop</a>");



  client.print("<a id='b12' href='?a=b12'");
  if (value == 12) {
    client.print("class='chosen'");
  }
  client.print(">Resume</a>");



  client.print("<a id='b13' href='?a=b13'");
  if (value == 13) {
    client.print("class='chosen'");
  }
  client.print(">1</a>");



  client.print("<a id='b14' href='?a=b14'");
  if (value == 14) {
    client.print("class='chosen'");
  }
  client.print(">2</a>");



  client.print("<a id='b15' href='?a=b15'");
  if (value == 15) {
    client.print("class='chosen'");
  }
  client.print(">3</a>");



  client.print("<a id='b16' href='?a=b16'");
  if (value == 16) {
    client.print("class='chosen'");
  }
  client.print(">4</a>");



  client.print("<a id='b17' href='?a=b17'");
  if (value == 17) {
    client.print("class='chosen'");
  }
  client.print(">5</a>");



  client.print("<a id='b18' href='?a=b18'");
  if (value == 18) {
    client.print("class='chosen'");
  }
  client.print(">6</a>");



  client.print("<a id='b19' href='?a=b19'");
  if (value == 19) {
    client.print("class='chosen'");
  }
  client.print(">7</a>");



  client.print("<a id='b20' href='?a=b20'");
  if (value == 20) {
    client.print("class='chosen'");
  }
  client.print(">8</a>");



  client.print("<a id='b21' href='?a=b21'");
  if (value == 21) {
    client.print("class='chosen'");
  }
  client.print(">9</a>");




  client.println("  </body> \
</html>");

  delay(1);
  #ifdef SERIAL_DEBUG
  Serial.println("Client disconnected");
  #endif
  Serial.println("");

}
