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

// uncomment the next line to turn on debugging
//#define DEBUGGING

#define AS5048_CMD_READ 0x4000
#define AS5048_REG_AGC 0x3FFD
#define AS5048_REG_MAG 0x3FFE
#define AS5048_REG_DATA 0x3FFF
#define AS5048_REG_ERR 0x1
#define AS5048_CMD_NOP 0x0

const int ledPin = 13;  // Status led
const int azPin = 5;
const int alPin = 15;
const int numSamples = 31;
const float smoothingFactor = 0.9;
const int numInitLoops = 20;

// the value of the current Azimuth and Altitude angle that we will report back to Sky Safari
int newAlAng = 0;
int newAzAng = 0;
// store the old angles for use in the smoothing of the sensor data
int oldAlAng = 0;
int oldAzAng = 0;

// the strings we send back to SkySafari
char azTics[] = "00000+";
char alTics[] = "00000+";

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
int readSensor(int cs);
void printWifiStatus();
byte calcEvenParity(word value);
int expSmooth(int oldVal, int newVal, float smoothingFactor);
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
    newAlAng = expSmooth(oldAlAng, rawData, smoothingFactor);
    rawData = readSensor(azPin) & 0x3FFF;
    oldAzAng = newAzAng;
    newAzAng = expSmooth(oldAzAng, rawData, smoothingFactor);
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
  word rawData = readSensor(alPin);
  // Data is in the bottom 14 bits
  rawData &= 0x3FFF;
  oldAlAng = newAlAng;
  newAlAng = expSmooth(oldAlAng, rawData, smoothingFactor);
  #ifdef DEBUGGING
    Serial.print("Raw Angle: ");
    Serial.print(ticsToAngle(rawData));
    Serial.print(rawData);
    Serial.print(" CircSmooth Al: ");
    Serial.print(aCircSmthAzAng);
    Serial.print("  expSmooth Al: ");
    Serial.print(newAlAng);
  #endif
  rawData = readSensor(azPin);
  rawData &= 0x3FFF;
  oldAzAng = newAzAng;
  newAzAng = expSmooth(oldAzAng, rawData, smoothingFactor);
  #ifdef DEBUGGING
    Serial.print("   Raw Angle: ");
    Serial.print(ticsToAngle(rawData));
    Serial.println(rawData);
    Serial.print("  expSmooth Az: ");
    Serial.print(" CircSmooth Az: ");
    Serial.println(aCircSmthAlAngle);
    Serial.println(newAzAng);
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
    itoa(newAlAng, tmp_string, 10);
    display.drawStr(55,36,tmp_string);
    itoa(newAzAng, tmp_string, 10);
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
        char encoderResponse[20];
        // pack the integers into the character array with tabs and returns
        sprintf(encoderResponse, "%i\t%i\r\n",newAlAng,newAzAng);
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
int readSensor(int cs){
  unsigned int data;
  pinMode(cs, OUTPUT);
  // Set up the command we will send
  word command = AS5048_CMD_READ | AS5048_REG_DATA;
  command |= calcEvenParity(command) <<15;
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
  // Drop cs low to enable the AS5048
  digitalWrite(cs, LOW);
  data = vspi->transfer16(command);
  digitalWrite(cs, HIGH);
  digitalWrite(cs, LOW);
  // you have to poll the chip twice.  Data from previous command comes
  // back on the next SPI transfer
  data = vspi->transfer16(command);
  digitalWrite(cs, HIGH);
  vspi->endTransaction();
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

///////////////////////////////////////////////////////////////////////////////
//
// Calculate Even parity of word
byte calcEvenParity(word value){
  byte count = 0;
  // loop through the 16 bits
  for (byte i = 0; i < 16; i++) {
    // if the rightmost bit is 1 increment our counter
    if (value & 0x1) {
      count++;
    }
    // shift off the rightmost bit
    value >>=1;
  }
  // all odd binaries end in 1
  return count & 0x1;
}
///////////////////////////////////////////////////////////////////////////////
//
// http://damienclarke.me/code/posts/writing-a-better-noise-reducing-analogread
//
///////////////////////////////////////////////////////////////////////////////
//
// return the exponentially smoothed value paying close attention to
// how we need to wrap the smoothing out around the transition from
// 2^14 back to 0
//
//
int expSmooth(int oldVal, int newVal, float smoothingFactor){
  int retVal = oldVal;
  // do some bounds checking
  if (smoothingFactor > 1){
    smoothingFactor = 1;
  } else if (smoothingFactor <0 ){
    smoothingFactor = 0;
  }
  // make sure we have not wrapped around
  // we do that by making sure that our old value is not more than half
  // of the total range away from the new value
  // For example:
  //    1 ---> 16383 is just 3 tic
  //  We want to do something reasonable with that sort of thing
  //
  if (oldVal - newVal > 8192){
    // we have wrapped from High to LOW
    // so move the new value out past the high end
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newVal += 16384;
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor)) % 16384;
  } else if (newVal - oldVal > 8192){
    // here we have wrapped from low to high
    // so move the new value to the low end ( may be negative but it still works)
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newVal -= 16384;
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor));
    // this could be negative.  If it is we have to wrap back to the top....
    if (retVal < 0){
      retVal += 16384;
    }
  } else {
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor));
  }
  return retVal;
}
