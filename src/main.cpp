/*
ESP_32 version of Digital Setting Circles project

Accept connections from SkySafari on port 12
Send Data back.

Version 0.3 - "Tidy is better"

 */
#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
#include <U8g2lib.h>            // U8g2 Library
#include <SPI.h>
#include "OneButton.h"
#include "secrets.h"
#include "AS5048.hpp"
#include "Event.hpp"

// uncomment the next line to turn on debugging
#define DEBUGGING

const int ledPin = 27;  // Status led
const int azPin = 5;    // SPI CS J1
const int alPin = 32;   // SPI CS J2
const int del = 100;

///////////////////////////////////////////////////////////////////////////////
//
// Function declaration
//
void printWifiStatus();
void buttonUpPress();
void buttonDownPress();
void buttonEnterPress();
void drawDisplay(int alAng, int azAng);
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// encoder objects

AS5048A alEnc(alPin);
AS5048A azEnc(azPin);

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

// display
// using HW I2C we only need to tell it the rotation
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R2);
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// Set up the WiFiServer
//
// Define this if you want to run as an Access Point.  If undefined it will connect to the
// SSID with the password below....
// 
// #define AP

const char *apssid = "ESPap";
const char *appassword = "gofish";

int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Server for SkySafari
WiFiServer server(4001);

boolean alreadyConnected = false; // whether or not the client was connected previously

// Events for our led
Event gEvent;

void ledOnEvent();
void ledOffEvent();
//
///////////////////////////////////////////////////////////////////////////////


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

  // the humble status led
  pinMode(ledPin, OUTPUT);
  ledOnEvent();

  // link the myClickFunction function to be called on a click event.   
  buttonUp.attachClick(buttonUpPress);
  buttonDown.attachClick(buttonDownPress);
  buttonEnter.attachClick(buttonEnterPress);

  // Set up networking
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
      // you're connected now, so print out the status:
      printWifiStatus();
    }
  #endif

  // Initialize the display
  display.begin();


  // start the angle server:
  server.begin();

  // initialize the encoders
  alEnc.init();
  azEnc.init();
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

  // read the encoders
  int alAng = alEnc.getAngle();
  int azAng = azEnc.getAngle();

  drawDisplay(alAng, azAng);

  // wait for a new client:
  WiFiClient thisClient = server.available();
  // when the client sends the first byte, say hello:
  while (thisClient) {
    if (!alreadyConnected) {
      alreadyConnected = true;
    }
    if (thisClient.connected()) {
      if (thisClient.available() > 0) {
        Serial.println(thisClient.read(),HEX);
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
  Serial.println("Button Up.");
  buttonUpCounter += 1;
}
void buttonDownPress(){
  buttonDownCounter += 1;
}
void buttonEnterPress(){
  buttonEnterCounter += 1;
}

void ledOnEvent(){
  digitalWrite(ledPin, HIGH);
  gEvent = Event(&ledOffEvent, millis() + 800);
}

void ledOffEvent(){
  gEvent = Event(&ledOnEvent, millis() + 600);
}

void drawDisplay(int alAng, int azAng){
  char tmp_string[12];
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
  itoa(alAng, tmp_string, 10);
  display.drawStr(55,36,tmp_string);
  itoa(azAng, tmp_string, 10);
  display.drawStr(55,48,tmp_string);
  display.sendBuffer();
}