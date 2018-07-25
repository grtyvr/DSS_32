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
//#include "AS5048A.h"
#include <SPI.h>

const int al = 5;    // altitiude Slave Select
const int az = 15;   // azimuth Slave Select
const int ledPin = 13;
const int buttonPin = 14;
// this could change in unexpected ways ( in an interupt )
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

static const int spiClk = 1000000; // 1 MHz
SPIClass * vspi = NULL;


///////////////////////////////////////////////////////////////////////////////
//
//  define functions for using the sensors
//
const int AS5048A_CLEAR_ERROR_FLAG              = 0x0001;
const int AS5048A_PROGRAMMING_CONTROL           = 0x0003;
const int AS5048A_OTP_REGISTER_ZERO_POS_HIGH    = 0x0016;
const int AS5048A_OTP_REGISTER_ZERO_POS_LOW     = 0x0017;
const int AS5048A_DIAG_AGC                      = 0x3FFD;
const int AS5048A_MAGNITUDE                     = 0x3FFE;
const int AS5048A_ANGLE                         = 0x3FFF;

SPISettings settings;

bool errorFlag;
byte _cs;
//byte cs;
//byte dout;
//byte din;
//byte clk;
word position;
//word transaction(word data);
/**
 * Utility function used to calculate even parity of word
 */
byte spiCalcEvenParity(word value){
	byte cnt = 0;
	byte i;

	for (i = 0; i < 16; i++)
	{
		if (value & 0x1)
		{
			cnt++;
		}
		value >>= 1;
	}
	return cnt & 0x1;
}

/*
 * Read a register from the sensor
 * Takes the address of the register as a 16 bit word
 * and the Clent Select pin
 * Returns the value of the register
 */
word readSPI(word registerAddress, byte arg_cs){
	_cs = arg_cs;
	word command = 0b0100000000000000; // PAR=0 R/W=R
	command = command | registerAddress;

	//Add a parity bit on the the MSB
	command |= ((word)spiCalcEvenParity(command)<<15);

	//Split the command into two bytes
	byte right_byte = command & 0xFF;
	byte left_byte = ( command >> 8 ) & 0xFF;

	#ifdef AS5048A_DEBUG
		Serial.print("Read (0x");
		Serial.print(registerAddress, HEX);
		Serial.print(") with command: 0b");
		Serial.println(command, BIN);
	#endif

	//SPI - begin transaction
	vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));

	//Send the command
	digitalWrite(_cs, LOW);
	vspi->transfer(left_byte);
	vspi->transfer(right_byte);
	digitalWrite(_cs,HIGH);

	//Now read the response
	digitalWrite(_cs, LOW);
	left_byte = vspi->transfer(0x00);
	right_byte = vspi->transfer(0x00);
	digitalWrite(_cs, HIGH);

	//SPI - end transaction
	vspi->endTransaction();

#ifdef AS5048A_DEBUG
	Serial.print("Read returned: ");
	Serial.print(left_byte, BIN);
	Serial.print(" ");
	Serial.println(right_byte, BIN);
#endif

	//Check if the error bit is set
	if (left_byte & 0x40) {
#ifdef AS5048A_DEBUG
		Serial.println("Setting error bit");
#endif
		errorFlag = true;
	}
	else {
		errorFlag = false;
	}

	//Return the data, stripping the parity and error bits
	return (( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF )) & ~0xC000;
}
/**
 * Returns the raw angle directly from the sensor
 */
word getRawRotation(byte arg_cs){
	_cs = arg_cs;
	return readSPI(AS5048A_ANGLE, _cs);
}
/**
 * Get the rotation of the sensor relative to the zero position.
 *
 * @return {int} between -2^13 and 2^13
 */
int getRotation(byte arg_cs){
	_cs = arg_cs;
	word data;
	int rotation;

	data = getRawRotation(_cs);
	rotation = (int)data - (int)position;
	if(rotation > 8191) rotation = -((0x3FFF)-rotation); //more than -180
	//if(rotation < -0x1FFF) rotation = rotation+0x3FFF;

	return rotation;
}
/*
 * Check if an error has been encountered.
 */
bool error(){
	return errorFlag;
}

/**
 * returns the value of the state register
 * @return 16 bit word containing flags
 */
word getState(byte arg_cs){
	_cs = arg_cs;
	return readSPI(AS5048A_DIAG_AGC, _cs);
}

/**
 * Print the diagnostic register of the sensor
 */
void printState(byte arg_cs){
	_cs = arg_cs;
	word data;

	data = getState(_cs);
	if(error()){
		Serial.print("Error bit was set!");
	}
	Serial.println(data, BIN);
}
/**
 * Returns the value used for Automatic Gain Control (Part of diagnostic
 * register)
 */
byte getGain(byte arg_cs){
	_cs = arg_cs;
	word data = getState(_cs);
	return (byte) data & 0xFF;
}
/*
 * Get and clear the error register by reading it
 */
word getErrors(byte arg_cs){
	_cs = arg_cs;
	return readSPI(AS5048A_CLEAR_ERROR_FLAG, _cs);
}

/*
 * Set the zero position
 */
void setZeroPosition(word arg_position){
	position = arg_position % 0x3FFF;
}

/*
 * Returns the current zero position
 */
word getZeroPosition(){
	return position;
}

/*
 * Write to a register
 * Takes the 16-bit  address of the target register, the 16 bit word of data
 * to be written to that register, and the ClientSelect pin
 * Returns the value of the register after the write has been performed. This
 * is read back from the sensor to ensure a sucessful write.
 */
word write(word registerAddress, word data, byte arg_cs) {
	_cs = arg_cs;
	word command = 0b0000000000000000; // PAR=0 R/W=W
	command |= registerAddress;

	//Add a parity bit on the the MSB
	command |= ((word)spiCalcEvenParity(command)<<15);

	//Split the command into two bytes
	byte right_byte = command & 0xFF;
	byte left_byte = ( command >> 8 ) & 0xFF;

#ifdef AS5048A_DEBUG
	Serial.print("Write (0x");
	Serial.print(registerAddress, HEX);
	Serial.print(") with command: 0b");
	Serial.println(command, BIN);
#endif

	//SPI - begin transaction
	vspi->beginTransaction(settings);
	//Start the write command with the target address
	digitalWrite(_cs, LOW);
	vspi->transfer(left_byte);
	vspi->transfer(right_byte);
	digitalWrite(_cs,HIGH);

	word dataToSend = 0b0000000000000000;
	dataToSend |= data;

	//Craft another packet including the data and parity
	dataToSend |= ((word)spiCalcEvenParity(dataToSend)<<15);
	right_byte = dataToSend & 0xFF;
	left_byte = ( dataToSend >> 8 ) & 0xFF;

#ifdef AS5048A_DEBUG
	Serial.print("Sending data to write: ");
	Serial.println(dataToSend, BIN);
#endif

	//Now send the data packet
	digitalWrite(_cs,LOW);
	vspi->transfer(left_byte);
	vspi->transfer(right_byte);
	digitalWrite(_cs,HIGH);

	//Send a NOP to get the new data in the register
	digitalWrite(_cs, LOW);
	left_byte = vspi->transfer(0x00);
	right_byte = vspi->transfer(0x00);
	digitalWrite(_cs, HIGH);

	//SPI - end transaction
	vspi->endTransaction();

	//Return the data, stripping the parity and error bits
	return (( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF )) & ~0xC000;
}
//
//
//  end of AS5048 calls
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  functions for dealing with calculations
//

///////////////////////////////////////////////////////////////////////////////
//
// Set up the display
// using HW I2C we only need to tell it the rotation
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);

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

// Define this if you want to run as an Access Point.  If undefined it will connect to the
// SSID with the password below....

#define AP
// uncomment the next line to turn on debugging
#define DEBUGGING

const char *apssid = "ESPap";
const char *appassword = "gofish";

int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(23);

boolean alreadyConnected = false; // whether or not the client was connected previously

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

void setup() {
//  sensor.init();
  // initialize an instance of the SPIClass attached to vspi
  vspi = new SPIClass(VSPI);

  vspi->begin();

  pinMode(al, OUTPUT);
  pinMode(az, OUTPUT);

  display.begin();
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
  //use the SPI buses
  // altitude sensor

  word valAlt = getRawRotation(al);
  Serial.print("Got Altitude of: 0x");
  Serial.println(valAlt, HEX);
  Serial.print("State: ");
  printState(al);
  Serial.print("Errors: ");
  Serial.println(getErrors(al));
  // Azimuth sensor

  word valAzt = getRawRotation(az);
  Serial.print("Got Azimuth of: 0x");
  Serial.println(valAzt, HEX);
  Serial.print("State: ");
  printState(az);
  Serial.print("Errors: ");
  Serial.println(getErrors(az));


  char tmp_string[8];
  long newPosition = myEnc.read();


  //String str = String(numberOfInterrupts);
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
    //
    display.drawStr(0,36, "Altitude: ");
    display.drawStr(0,48, " Azimuth: ");
    display.setDrawColor(0);
    display.drawBox(55,24,24,24);
    display.setDrawColor(1);
    itoa(valAlt, tmp_string, 10);
    display.drawStr(55,36,tmp_string);
    itoa(valAzt, tmp_string, 10);
    display.drawStr(55,48,tmp_string);

//    display.drawStr(60,24,str);
  } while ( display.nextPage() );

  // wait for a new client:
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
        thisClient.print("00000");
        thisClient.print("\t");
        thisClient.print("00000");
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
  }


  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}
