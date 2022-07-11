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
#include "WiFi.h"
#include "Network.hpp"

namespace WebConfigServer {

WebServer *configServer = NULL;
String channel = "1";
String address = "4";

IPAddress myIP = WiFi.softAPIP();
String ipAddr = myIP.toString();
uint8_t addr = myIP[2];

void initialize(){
  const int portNumber = 80; 
  configServer = new WebServer(portNumber);
  // register handlers
  configServer->on("/", handle_OnConnect);
  configServer->on("/restartNetwork", handle_restartNetwork);
  configServer->onNotFound(handle_NotFound);

  configServer->begin();
  Serial.print("WebConfig listening on port: ");
  Serial.println(portNumber);
}

void processClient(){
  configServer->handleClient();
}


void handle_OnConnect(){
  String message = "URI: ";
  message += configServer->uri();
  message += "\nMethod: ";
  message += (configServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += configServer->args();
  message += "\n";
  for (uint8_t i =  0; i < configServer->args(); i++){
    message += " " + configServer->argName(i) + ": " + configServer->arg(i) + "\n";
  }
  Serial.println(message);
  IPAddress myIP = WiFi.softAPIP();
  String IPaddr = String(myIP[2]);
  configServer->send(200, "text/html", SendHTML(channel, IPaddr));
}

void handle_restartNetwork(){
  String message = "URI: ";
  message += configServer->uri();
  message += "\nMethod: ";
  message += (configServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += configServer->args();
  message += "\n";
  for (uint8_t i =  0; i < configServer->args(); i++){
    message += " " + configServer->argName(i) + ": " + configServer->arg(i) + "\n";
  }
  Serial.println(message);
  channel = configServer->arg(0);
  ipAddr = configServer->arg(1);
  Serial.println("reconfiguring networking");
  Serial.print("setting IP Address to: 192.168.");
  Serial.print(ipAddr);
  Serial.println(".1");
  int ip = ipAddr.toInt();
  IPAddress newAddr(192, 168, ip, 1);
  IPAddress netmask(255,255,255,0);
  Serial.println(newAddr);
  WiFi.softAPConfig(newAddr, newAddr, netmask);
  delay(100);
  Serial.print("setting WiFi channel to: ");
  Serial.println(channel);
  Serial.println(WiFi.softAPSSID());
  WiFi.softAP("DSC_ESP32_AP", "", channel.toInt());
  delay(100);
  configServer->send(200, "text/html", SendHTML(channel, ipAddr));
}

void handle_NotFound(){
  String message = "File not found\n\n";
  message += "URI: ";
  message += configServer->uri();
  message += "\nMethod: ";
  message += (configServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += configServer->args();
  message += "\n";
  for (uint8_t i =  0; i < configServer->args(); i++){
    message += " " + configServer->argName(i) + ": " + configServer->arg(i) + "\n";
  }
  configServer->send(404, "text/plain", message);
}

String SendHTML(String channel, String addr){
  String pageString = "<!DOCTYPE html> <html>\n";
  pageString += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  pageString += "<title>Configuration Page</title>\n";
  pageString += "</head>\n";
  pageString += "<body>\n";
  pageString +="<h1>ESP32 Digital Setting Circles Configuration</h1>\n";
  pageString +="<p>Select the WiFi Channel.</p>\n";
  pageString += "<form action=""/restartNetwork"" method=""POST"">";
  pageString +="<label for=""channel"">Channel:</label>";
  pageString +="<select id=""channel"" name=""channel"">";
    pageString +="<option value=""1"" ";
    if (channel == "1") {
      pageString += "selected";
    }
    pageString += ">1</option>";
    
    pageString += "<option value=""2"" ";
    if (channel == "2") {
      pageString += "selected";
    }
    pageString += ">2</option>";
    
    pageString += "<option value=""3"" ";
    if (channel == "3") {
      pageString += "selected";
    }
    pageString += ">3</option>";
    
    pageString +="<option value=""4"" ";
    if (channel == "4") {
      pageString += "selected";
    }
    pageString += ">4</option>";
    
    pageString +="<option value=""5"" ";
      if (channel == "5") {
      pageString += "selected";
    }
    pageString += ">5</option>";

    pageString +="<option value=""6"" ";
      if (channel == "6") {
      pageString += "selected";
    }
    pageString += ">6</option>";

    pageString +="<option value=""7"" ";
      if (channel == "7") {
      pageString += "selected";
    }
    pageString += ">7</option>";

    pageString +="<option value=""8"" ";
    if (channel == "8") {
      pageString += "selected";
    }
    pageString += ">8</option>";

    pageString +="<option value=""9"" ";
      if (channel == "9") {
      pageString += "selected";
    }
    pageString += ">9</option>";

    pageString +="<option value=""10"" ";
    if (channel == "10") {
      pageString += "selected";
    }
    pageString += ">10</option>";
    
    pageString +="<option value=""11"" ";
    if (channel == "11") {
      pageString += "selected";
    }
    pageString += ">11</option>";
    
    pageString +="<option value=""12"" ";
    if (channel == "12") {
      pageString += "selected";
    }
    pageString += ">12</option>";
  pageString +="</select>";

  // Selecting the IP Address
  pageString +="<p>Select the IP Address to use.</p>\n";
  pageString +="<label for=""IPAddress"">IP Address:</label>";
  pageString +="<select id=""IPAddress"" name=""IPAddress"">";
    pageString +="<option value=""1"" ";
    if (addr == "1") {
      pageString += "selected";
    }
    pageString += ">192.168.1.1</option>";
    
    pageString += "<option value=""2"" ";
    if (addr == "2") {
      pageString += "selected";
    }
    pageString += ">192.168.2.1</option>";
    
    pageString += "<option value=""3"" ";
    if (addr == "3") {
      pageString += "selected";
    }
    pageString += ">192.168.3.1</option>";
    
    pageString +="<option value=""4"" ";
    if (addr == "4") {
      pageString += "selected";
    }
    pageString += ">192.168.4.1</option>";
    
    pageString +="<option value=""5"" ";
      if (addr == "5") {
      pageString += "selected";
    }
    pageString += ">192.168.5.1</option>";

    pageString +="<option value=""6"" ";
      if (addr == "6") {
      pageString += "selected";
    }
    pageString += ">192.168.6.1</option>";

    pageString +="<option value=""7"" ";
      if (addr == "7") {
      pageString += "selected";
    }
    pageString += ">192.168.7.1</option>";

    pageString +="<option value=""8"" ";
    if (addr == "8") {
      pageString += "selected";
    }
    pageString += ">192.168.8.1</option>";

    pageString +="<option value=""9"" ";
      if (addr == "9") {
      pageString += "selected";
    }
    pageString += ">192.168.9.1</option>";

    pageString +="<option value=""10"" ";
    if (addr == "10") {
      pageString += "selected";
    }
    pageString += ">192.168.10.1</option>";
    
    pageString +="<option value=""11"" ";
    if (addr == "11") {
      pageString += "selected";
    }
    pageString += ">192.168.11.1</option>";
    
    pageString +="<option value=""12"" ";
    if (addr == "12") {
      pageString += "selected";
    }
    pageString += ">192.168.12.1</option>";
  pageString +="</select>\n\n";

  pageString += "<p>Save Settings</p>";
  pageString += "<input type=""submit"" name=""Save and Restart Network"">";
  pageString +="<form>\n";

  pageString += "<p>Note that you will have to reconnect to the network and";
  pageString += " you will have to use the new IP Address to access this page after the restart.</p>\n";
  pageString +="</body>\n";
  pageString +="</html>\n";

  return pageString;
}

} // namespace WebConfigServer