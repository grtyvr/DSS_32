/*
ESP_32 version of Digital Setting Circles project

Accept connections from SkySafari on port 12
Send Data back.

Version 0.3 - "Tidy is better"

 */
#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
//#include <Encoder.h>            // Paul Stoffregen Rotary Encoder Library
#include <U8g2lib.h>            // U8g2 Library
#include <SPI.h>
#include "OneButton.h"
#include "secrets.h"

// uncomment the next line to turn on debugging
#define DEBUGGING

const int ledPin = 27;  // Status led
const int azPin = 5;    // SPI CS J1
const int alPin = 36;   // SPI CS J2
const int del = 100;

///////////////////////////////////////////////////////////////////////////////
//
// Function declaration
//
void printWifiStatus();
void buttonUpPress();
void buttonDownPress();
void buttonEnterPress();
//
///////////////////////////////////////////////////////////////////////////////

// // the value of the current Azimuth and Altitude angle that we will report back to Sky Safari
// int newAlAng = 0;
// int newAzAng = 0;
// // store the old angles for use in the smoothing of the sensor data
// int oldAlAng = 0;
// int oldAzAng = 0;

///////////////////////////////////////////////////////////////////////////////
//
// Set up the UI 
//
// Buttons
const int buttonUpPin = 4;
const int buttonEnterPin = 16;
const int buttonDownPin = 17;

OneButton buttonUp(buttonUpPin, true, true);
OneButton buttonDown(buttonDownPin, true, true);
OneButton buttonEnter(buttonEnterPin, true, true);

volatile int buttonUpCounter = 0;
volatile int buttonDownCounter = 0;
volatile int buttonEnterCounter = 0;

unsigned long previousMillis = 0;
int ledState = LOW;             // ledState used to set the LED

// Set up the display
// using HW I2C we only need to tell it the rotation
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set up the WiFiServer
//
// Define this if you want to run as an Access Point.  If undefined it will connect to the
// SSID with the password below....
// 
#define AP

const char *apssid = AP_NAME;
const char *appassword = AP_PWD;

int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(23);
WiFiServer webServer(80);

boolean alreadyConnected = false; // whether or not the client was connected previously
boolean webClientConnected = false;
//


///////////////////////////////////////////////////////////////////////////////
//
// Set up SPI bus
//
static const int spiClk = 1000000; // 1 MHz
SPIClass * vspi = NULL;
//
///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//*
//* Setup
//*
//*****************************************************************************
void setup() {
  //Initialize serial
  Serial.begin(115200);
  delay(500);

  pinMode(ledPin, OUTPUT);

  // link the myClickFunction function to be called on a click event.   
  buttonUp.attachClick(buttonUpPress);
  buttonDown.attachClick(buttonDownPress);
  buttonEnter.attachClick(buttonEnterPress);

  #ifdef AP
    Serial.println("Setting up WiFi Access Point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apssid);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  #else
    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);

      // wait 5 seconds for connection:
      delay(5000);
    }
  #endif

  display.begin();

  // start the angle server:
  server.begin();
  webServer.begin();
  // you're connected now, so print out the status:
  #ifdef AP
  #else
    printWifiStatus();
  #endif
} // end setup


///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//*
//*                   LOOP
//*
//*
void loop() {
  unsigned long now = millis();
  delay(10);
  // keep watching the push button:
  buttonUp.tick();
  buttonDown.tick();
  buttonEnter.tick();
  
//  ucg.setFont(ucg_font_ncenR14r);
//  ucg.setPrintPos(0,25);
//  ucg.setColor(255, 255, 255);
//  ucg.print("Hello World!");

  // word rawData = readSensor(alPin);
  // // Data is in the bottom 14 bits
  // rawData &= 0x3FFF;
  // oldAlAng = newAlAng;
  // newAlAng = expSmooth(oldAlAng, rawData, smoothingFactor);
  // // let's slow down the main loop a bit.  See if that helps.
  // //
  // delay(del);
  // #ifdef DEBUGGING
  //   Serial.print("Raw Angle: ");
  //   //Serial.print(ticsToAngle(rawData));
  //   Serial.print(rawData);
  //   //Serial.print(" CircSmooth Al: ");
  //   //Serial.print(aCircSmthAzAng);
  //   Serial.print("  expSmooth Al: ");
  //   Serial.print(newAlAng);
  // #endif
  // rawData = readSensor(azPin);
  // rawData &= 0x3FFF;
  // oldAzAng = newAzAng;
  // newAzAng = expSmooth(oldAzAng, rawData, smoothingFactor);
  // delay(del);
  #ifdef DEBUGGING
    // Serial.print("   Raw Angle: ");
    // //Serial.print(ticsToAngle(rawData));
    // Serial.println(rawData);
    // Serial.print("  expSmooth Az: ");
    // //Serial.print(" CircSmooth Az: ");
    // //Serial.println(aCircSmthAlAngle);
    // Serial.println(newAzAng);
  #endif

  char tmp_string[12];
//  long newPosition = myEnc.read();

//  display.firstPage();
//  do {
    display.clearBuffer();
    display.setFont(u8g2_font_courB08_tf);

    display.drawStr(0,12,"Up:");
    int increment = display.getStrWidth("Up:"); 
    itoa(buttonUpCounter, tmp_string, 10);
    display.drawStr(increment + 3,12,tmp_string);

    display.drawStr(40,12,"Ok:");
    increment = display.getStrWidth("OK:"); 
    itoa(buttonEnterCounter, tmp_string, 10);
    display.drawStr(40 + increment + 3,12,tmp_string);

    display.drawStr(80,12,"Dn:");
    increment = display.getStrWidth("Dn:"); 
    itoa(buttonDownCounter, tmp_string, 10);
    display.drawStr(80 + increment + 3,12,tmp_string);  

    display.setDrawColor(1);
    display.drawStr(0,36, "Altitude: ");
    display.drawStr(0,48, " Azimuth: ");
    display.setDrawColor(0);
    display.drawBox(55,24,24,24);
    display.setDrawColor(1);
//    itoa(newAlAng, tmp_string, 10);
//    display.drawStr(55,36,tmp_string);
//    itoa(newAzAng, tmp_string, 10);
//    display.drawStr(55,48,tmp_string);
    display.sendBuffer();
//  } while ( display.nextPage() );

  // wait for a new client:
  WiFiClient thisClient = server.available();
  // when the client sends the first byte, say hello:
  while (thisClient) {
    if (!alreadyConnected) {
      alreadyConnected = true;
    }
    if (thisClient.connected()) {
      if (thisClient.available() > 0) {
        Serial.println(thisClient.read());
        char encoderResponse[20];
        // pack the integers into the character array with tabs and returns
        sprintf(encoderResponse, "%i\t%i\r\n",0,0);
        thisClient.println(encoderResponse);
        #ifdef DEBUGGING
          Serial.println(encoderResponse);
        #endif
        // discard remaining bytes
        thisClient.flush();
      }
    }
    else {
      thisClient.stop();
      alreadyConnected = false;
    }
  } // end this client
  WiFiClient webClient = webServer.available();
  if (webClient) {
    Serial.println("New Web Client");
    String currentLine = "";
    if (!webClientConnected) {
      webClientConnected = true;
    }
    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        Serial.write(c);
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            webClient.println("HTTP/1.1 200 OK");
            webClient.println("Content-type:text/html");
            webClient.println();

            // the content of the HTTP response follows the header:
            webClient.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            webClient.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            webClient.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

/*
        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(33, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(33, LOW);                // GET /L turns the LED off
        }
*/
      }
    }
    // close the connection:
    webClient.stop();
    Serial.println("Client Disconnected.");
}
// blink status led
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500){
    previousMillis = currentMillis;
    if (ledState == LOW){
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  }
  digitalWrite(ledPin, ledState);
}
//*  end loop
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Function Definitions
//
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void buttonUpPress(){
  buttonUpCounter += 1;
}
void buttonDownPress(){
  buttonDownCounter += 1;
}
void buttonEnterPress(){
  buttonEnterCounter += 1;
}