/*
ESP_32 version of Digital Setting Circles project

Accept connections from SkySafari on port 12
Send Data back.

Thanks to the folks at ZoetropeLabs
https://github.com/ZoetropeLabs/AS5048A-Arduino

Thanks to Paul Stoffregen for the encoder library
Thanks to Adafruit for being so AWESOME!  And for the graphics libraries.

 */
#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Encoder.h>            // Paul Stoffregen Rotary Encoder Library
#include <U8g2lib.h>            // U8g2 Library
#include <SPI.h>
#include "sensors.h"

const int ledPin = 13;  // Status led
const int azPin = 5;
const int alPin = 15;
const int numSamples = 31;
const float smoothingFactor = 0.9;
const int numInitLoops = 20;

// the value of the current Azimuth and Altitude angle that we will report back to Sky Safari
int newAzAng = 0;
int newAlAng = 0;
// store the old angles for use in the smoothing of the sensor data
int oldAzAng = 0;
int oldAzAng = 0;

// the strings we send back to SkySafari
char azTics[] = "00000+";
char alTics[] = "00000+";

// uncomment the next line to turn on debugging
//#define DEBUGGING

///////////////////////////////////////////////////////////////////////////////
//
// Set up the rotary encoder stuff
//
const int buttonPin = 14;
// this could change in unexpected ways ( in an interupt )
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;
Encoder myEnc(4,0);

// Set up interupt
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

long oldPosition  = -999;
unsigned long previousMillis = 0;
int ledState = LOW;             // ledState used to set the LED
int buttonState = 0;
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set up the display
// using HW I2C we only need to tell it the rotation
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);

///////////////////////////////////////////////////////////////////////////////
//
// Set up the WiFiServer
//
// Define this if you want to run as an Access Point.  If undefined it will connect to the
// SSID with the password below....
#define AP

const char *apssid = "ESPap";
const char *appassword = "gofish";

int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(23);
boolean alreadyConnected = false; // whether or not the client was connected previously
//
///////////////////////////////////////////////////////////////////////////////
//
// Function declaration
//
unsigned int readSensor(int cs);
void printWifiStatus();

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Set up SPI bus
//
static const int spiClk = 1000000; // 1 MHz
SPIClass * vspi = NULL;
//
///////////////////////////////////////////////////////////////////////////////


void setup() {
  display.begin();  // throw up a splash screen here  ???
  // initialize an instance of the SPIClass attached to vspi
  vspi = new SPIClass(VSPI);
  // Wake up the bus
  vspi->begin();
  // fill up our smoothing arrays with data
  for (int i = 0; i < numInitLoops; i++){
    // Store the bottom 14 bits
    word rawData = readSensor(alPin) & 0x3FFF;
    oldAlAng = newAlAng;
    newAlAng = expSmooth(oldAlAng, newAlAng, smoothingFactor);
    rawData = readSensor(azPin) & 0x3FFF;
    oldAzAng = newAzAng;
    newAzAng = expSmooth(oldAzAng, newAzAng, smoothingFactor);
  }

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLDOWN);
  // trigger the interrupt on falling edge
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleInterrupt, RISING);

  //Initialize serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("Basic Encoder Test:");

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

  // start the server:
  server.begin();
  // you're connected now, so print out the status:
  #ifdef AP
  #else
    printWifiStatus();
  #endif

} // end setup

void loop() {
  word rawData = readSensor(azPin);
  // Discard the top two and bottom bit
  rawData &= 0x3FFE;
  aCircSmthAzAng = aCircSmth(ticsToAng(rawData), 1, aCircSmthAzAng, smthAzAngs);
  #ifdef DEBUGGING
    Serial.print("Raw Angle: ");
    Serial.print(ticsToAngle(rawData));
    Serial.print(" CircSmooth Az: ");
    Serial.print(aCircSmthAzAng);
  #endif
  // Discard the top two and bottom bit
  rawData = readSensor(alPin);
  rawData &= 0x3FFE;
  aCircSmthAlAng = aCircSmth(ticsToAng(rawData), 2, aCircSmthAlAng, smthAlAngs);
  #ifdef DEBUGGING
    Serial.print("Raw Angle: ");
    Serial.print(ticsToAngle(rawData));
    Serial.print(" CircSmooth Al: ");
    Serial.println(aCircSmthAlAngle);
  #endif

  char tmp_string[12];
  long newPosition = myEnc.read();

  display.firstPage();
  do {
    display.setFont(u8g2_font_courB08_tf);
    //display.drawStr(0,12,"Hello World!");
    display.drawStr(0,12,"Button Presses  : ");
    display.drawStr(0,24,"Encoder Position: ");
    display.setDrawColor(0);
    display.drawBox(105,0,24,24);
    display.setDrawColor(1);
    itoa(numberOfInterrupts, tmp_string, 10);
    display.drawStr(105,12,tmp_string);
    itoa(newPosition, tmp_string, 10);
    display.drawStr(105,24,tmp_string);
    display.drawStr(0,36, "Altitude: ");
    display.drawStr(0,48, " Azimuth: ");
    display.setDrawColor(0);
    display.drawBox(55,24,24,24);
    display.setDrawColor(1);
    dtostrf(aCircSmthAlAng,10,2,tmp_string);
//    itoa(advancedCircularSmoothAltitudeAngle, tmp_string, 10);
    display.drawStr(55,36,tmp_string);
    dtostrf(aCircSmthAzAng,10,2,tmp_string);
//    itoa(advancedCircularSmoothAzimuthAngle, tmp_string, 10);
    display.drawStr(55,48,tmp_string);
  } while ( display.nextPage() );

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
        // if there are chars to read....
        // lets print a response and discard the rest of the bytes
        thisClient.print(padTic(angToTics(aCircSmthAzAng), "+"));
        thisClient.print("\t");
        thisClient.print(padTic(angToTics(aCircSmthAlAng), "+"));
        thisClient.print("\r\n");
        #ifdef DEBUGGING
          Serial.print("Azimuth tic: ");
          Serial.print("00000+");
          Serial.print(" Altitude tic: ");
          Serial.println("00000+");
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
// Update the rotary Encoder

  if(interruptCounter > 0) {
    // Since we do not have access to NoInterrupts and Interrupts yet
    // we use the portENTER_CRITICAL portEXIT_CRITICAL
    portENTER_CRITICAL(&mux);
    interruptCounter--;
    portEXIT_CRITICAL(&mux);

    // handle interrupt
    numberOfInterrupts++;
    Serial.print("Button press.  Total: ");
    Serial.println(numberOfInterrupts);
  }

  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}
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
///////////////////////////////////////////////////////////////////////////////
//
// Read the sensor REG_DATA register
//
// ToDo:  This is a 32 bit chip.  Why!? am i splitting into high and low byte
//         rewrite this to use transfer16()
unsigned int readSensor(int cs){
  unsigned int data;
  unsigned int cmdHbyte;
  unsigned int cmdLbyte;
  unsigned int datHbyte;
  unsigned int datLbyte;
  pinMode(cs, OUTPUT);
  word command = AS5048_CMD_READ | AS5048_REG_DATA;                        // Set up the command we will send
  command |= calcEvenParity(command) <<15;                            // assign the parity bit
  cmdHbyte = highByte(command);                                   // split it into bytes
  cmdLbyte = lowByte(command);                                     //
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
  digitalWrite(cs, LOW);                                             // Drop ssl to enable the AS5048's
  datHbyte = vspi->transfer(cmdHbyte);                         // send the initial read command
  datLbyte = vspi->transfer(cmdLbyte);
  digitalWrite(cs, HIGH);                                            // disable the AS5048's
  digitalWrite(cs, LOW);                                             // Drop ssl to enable the AS5048's
  datHbyte = vspi->transfer(cmdHbyte);                         // send the initial read command
  datLbyte = vspi->transfer(cmdLbyte);
  digitalWrite(cs, HIGH);                                            // disable the AS5048's
  vspi->endTransaction();
  data = datHbyte;                                               // Store the high byte in my 16 bit varriable
  data = data << 8;                                                   // shift left 8 bits
  data = data | datLbyte;                                         // tack on the low byte
  #ifdef DEBUGGING
    Serial.println();
    Serial.print("Sent Command: ");
    Serial.println(command, BIN);
    Serial.print("To register: ");
    Serial.println(AS5048_REG_DATA, BIN);
    Serial.println(data);
  #endif
  return data;
}
