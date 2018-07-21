/*
ESP_32 version of Digital Setting Circles project

Accept connections from SkySafari on port 12
Send Data back.

Thanks to the folks at ZoetropeLabs
https://github.com/ZoetropeLabs/AS5048A-Arduino

Thanks to Paul Stoffregen for the encoder library
Thanks to Adafruit for being so AWESOME!  And for the graphics libraries.

 */

#include <Arduino.h>
#include <WiFi.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AS5048A.h>

Encoder myEnc(4,0);

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const int ledPin = 13;
const int buttonPin = 2;
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

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

void testdrawline() {
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }
  delay(250);
}

void setup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // draw many lines
  testdrawline();
  display.display();
  delay(2000);
  display.clearDisplay();

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
  // wait for a new client:
  //delay(1000);
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

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}

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
