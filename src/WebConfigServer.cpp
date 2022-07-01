//
// Web Configuration Server
// ---------------------------------------------------------------------------
// (c)2021 by GRTYVR. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "WebConfigServer.hpp"
#include "WebServer.h"
#include "Network.hpp"

namespace WebConfigServer {

WebServer *configServer = NULL;
uint8_t LED1pin = 4;
bool LED1status = LOW;

uint8_t LED2pin = 5;
bool LED2status = LOW;

void initialize(){
  const int portNumber = 80; 
  configServer = new WebServer(portNumber);
  // register handlers
  configServer->on("/", handle_OnConnect);
  configServer->on("/led1On", handle_led1On);
  configServer->on("/led1Off", handle_led1Off);
  configServer->on("/led2On", handle_led2On);
  configServer->on("/led2Off", handle_led2Off);
  configServer->onNotFound(handle_NotFound);

  configServer->begin();
  Serial.print("WebConfig listening on port: ");
  Serial.println(portNumber);
}

void processClient(){
  configServer->handleClient();
}

void handle_SetChannel(int channnel){
  Serial.println("Setting channel to: ");
}

void handle_led1On(){
  LED1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  configServer->send(200, "text/html", SendHTML(true,LED2status));
}

void handle_led1Off(){
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  configServer->send(200, "text/html", SendHTML(false,LED2status));
}
void handle_led2On(){
  LED2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  configServer->send(200, "text/html", SendHTML(LED1status,true));
}
void handle_led2Off(){
  LED2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  configServer->send(200, "text/html", SendHTML(LED1status,false));
}

void handle_OnConnect(){
  LED1status = LOW;
  LED2status = LOW;
  configServer->send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_NotFound(){
  String message = "File not found\n\n";
  message += "URI: ";
  message += configServer->uri();
  message += "\nMethod: ";
  message += (configServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += configServer->args();
  for (uint8_t i =  0; i < configServer->args(); i++){
    message += " " + configServer->argName(i) + ": " + configServer->arg(i) + "\n";
  }
  configServer->send(404, "text/plain", message);
}

String SendHTML(uint8_t led1Status, uint8_t led2Status){
  String pageString = "<!DOCTYPE html> <html>\n";
  pageString += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  pageString += "<title>Configuration Page</title>\n";
  // CSS to style the on/off buttons 
  // Feel free to change the background-color and font-size attributes to fit your preferences
  pageString += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  pageString += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;";
  pageString += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  pageString += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  pageString += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  pageString += ".button-on {background-color: #3498db;}\n";
  pageString += ".button-on:active {background-color: #2980b9;}\n";
  pageString += ".button-off {background-color: #34495e;}\n";
  pageString += ".button-off:active {background-color: #2c3e50;}\n";
  pageString += "p {font-size: 14px; color: #888; margin_bottom: 10px;}\n";
  pageString += "</style>\n";
  pageString += "</head>\n";
  pageString += "<body>\n";
  pageString +="<h1>ESP32 Digital Setting Circles Configuration</h1>\n";
  pageString += "<form action=""/"" method=""POST"">";
  pageString +="<label for=""channel"">Select Channel:</label>";
  pageString +="<select id=""channel"" name=""channel"">";
  pageString +="<option value=""1"">1</option>";
  pageString +="<option value=""2"">2</option>";
  pageString +="<option value=""3"">3</option>";
  pageString +="<option value=""4"">4</option>";
  pageString +="<option value=""5"">5</option>";
  pageString +="<option value=""6"">6</option>";
  pageString +="<option value=""7"">7</option>";
  pageString +="<option value=""8"">8</option>";
  pageString +="<option value=""9"">9</option>";
  pageString +="<option value=""10"">10</option>";
  pageString +="<option value=""11"">11</option>";
  pageString +="<option value=""12"">13</option>";
  pageString +="</select>";
  pageString +="<input type=""submit"" name=""Save Settings"">";
  pageString +="<form>\n";
  if(led1Status){
    pageString +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
    }
  else {
    pageString +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";
    }
  if(led2Status) {
    pageString +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
    }
  else {
    pageString +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";
    }

  pageString +="</body>\n";
  pageString +="</html>\n";

  return pageString;
}

} // namespace WebConfigServer