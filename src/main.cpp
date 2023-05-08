/*
ESP_32 version of Digital Setting Circles project

Accept connections from SkySafari on port 12
Send Data back.

Version 0.3 - "Tidy is better"

 */
#include <stdlib.h>
#include <Arduino.h>
#include "Buttons.hpp"
#include "Loop.hpp"
#include "Display.hpp"
#include "Network.hpp"
#include "AngleServer.hpp"
#include "Application.hpp"
#include "WebConfigServer.hpp"
#include "Preferences.h"

using namespace lr;
using namespace grt;

Preferences preferences;

/// The Event Loop
///
event::BasicLoop<event::StaticStorage<16>> gEventLoop;

// uncomment the next line to turn on debugging
//#define DEBUGGING

const char *apssid = "DSC_Circus_Cannon";
const int ledPin = 27;  // Status led
int al_max_tics = 9544;
int az_max_tics = 5216;

///////////////////////////////////////////////////////////////////////////////
//
// Function declaration
//
// void processButtonPresses();
// void processClientRequest();
void ledOnEvent();
void ledOffEvent();
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set up the UI 
//
// volatile int buttonUpCounter = 0;
// volatile int buttonDownCounter = 0;
// volatile int buttonEnterCounter = 0;
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//*
//* Setup
//*
//*****************************************************************************
void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1500);

  preferences.begin("prefs", false);
  int storedAlMaxTics = preferences.getInt("alMaxTics", al_max_tics);
  int storedAzMaxTics = preferences.getInt("azMaxTics", az_max_tics);
  if (storedAlMaxTics != al_max_tics) {
    al_max_tics = storedAlMaxTics;
  }
  if (storedAzMaxTics != az_max_tics){
    az_max_tics = storedAzMaxTics;
  }
  preferences.end();
  // the humble status led
  pinMode(ledPin, OUTPUT);
  ledOnEvent();
  
  // Buttons
//  Buttons::initialize();
//  event::mainLoop().addPollEvent(&Application::processButtonPresses);

  // Initialize the display
 Display::initialize();
 event::mainLoop().addRepeatedEvent(&Display::update,60_ms); // about 16 FPS

  // start the network
  Network::initialize(apssid);

  // start the Encoders
  Encoders::initialize();
  event::mainLoop().addRepeatedEvent(&Encoders::update,10_ms);

  // start the angleServer and poll for clients
  AngleServer::initialize();
  event::mainLoop().addPollEvent(&AngleServer::processClient);

  // start the WebConfigServer and poll for clients
  WebConfigServer::initialize();
  event::mainLoop().addPollEvent(&WebConfigServer::processClient);

} // end setup

///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//*
//*                   LOOP
//*
void loop() {
  // Process our event loop
  gEventLoop.loopOnce();
}
//*  end loop
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Function Definitions
//

void ledOnEvent(){
  digitalWrite(ledPin, HIGH);
  event::mainLoop().addDelayedEvent(&ledOffEvent, 800_ms);
}

void ledOffEvent(){
  digitalWrite(ledPin, LOW);
  event::mainLoop().addDelayedEvent(&ledOnEvent, 600_ms);
}
