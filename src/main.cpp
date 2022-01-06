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

using namespace lr;
using namespace grt;

/// The Event Loop
///
event::BasicLoop<event::StaticStorage<16>> gEventLoop;

// uncomment the next line to turn on debugging
//#define DEBUGGING

const int ledPin = 27;  // Status led

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

  // the humble status led
  pinMode(ledPin, OUTPUT);
  ledOnEvent();
  
  // Buttons
  Buttons::initialize();
  event::mainLoop().addPollEvent(&Application::processButtonPresses);

  // Initialize the display
  Display::initialize();
  event::mainLoop().addRepeatedEvent(&Display::update,60_ms); // about 16 FPS

  // start the network
  Network::initialize();

  Encoders::initialize();

  // start the angleServer
  AngleServer::initialize();
  event::mainLoop().addPollEvent(&AngleServer::processClient);

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

/// Process all button presses.
///
// void processButtonPresses()
// {
//     switch (Buttons::getNextButtonPress()) {
//     case Buttons::Up:
//         buttonUpCounter += 1;
//         break;
//     case Buttons::OK:
//         buttonEnterCounter += 1;
//         break;
//     case Buttons::Down:
//         buttonDownCounter += 1;
//     default:
//         break;
//     }
// }

void ledOnEvent(){
  digitalWrite(ledPin, HIGH);
  event::mainLoop().addDelayedEvent(&ledOffEvent, 800_ms);
}

void ledOffEvent(){
  digitalWrite(ledPin, LOW);
  event::mainLoop().addDelayedEvent(&ledOnEvent, 600_ms);
}
