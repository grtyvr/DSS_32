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
#include "Encoders.hpp"
//#include <string>



namespace grt{
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
  configServer->on("/configureEncoders", handle_configureEncoders);
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

void handle_configureEncoders(){
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
  Encoders::Position maxTics;
  maxTics.azimuth = configServer->arg(0).toInt();
  maxTics.altitude = configServer->arg(1).toInt();
  Encoders::setMaxTics(maxTics);
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
  Serial.println(message);
  configServer->send(404, "text/plain", message);
}

String SendHTML(String channel, String addr){
  Encoders::Position curMaxTics = Encoders::getMaxTics();
  String maxAz = String(curMaxTics.azimuth);
  String maxAl = String(curMaxTics.altitude);
  String pgStr = "";
  pgStr += "<!DOCTYPE html>";
  pgStr += "  <html lang=\"en\">";
  pgStr += "    <head>";
  pgStr += "      <meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  pgStr += "      <title>DSC Configuration</title>";
  pgStr += "      <style>";
  pgStr += getCSSSection();
  pgStr += "      </style>";
  pgStr += "    </head>";

  pgStr += "    <body>";
  pgStr += "      <header class=\"page-header\">";
  pgStr += "        Merope DSC Configuration";
  pgStr += "      </header>";
  pgStr += "      <nav>";
  pgStr += "        <a href=\"#networking\">Networking</a>";
  pgStr += "        <a href=\"#encoders\">Encoders</a>";
  pgStr += "      </nav>";
  pgStr += "      <div class=\"config-container\">";
  pgStr += "        <section id=\"networking\">";
  pgStr += "          <div class=\"config-pane\">";
  pgStr += "            <div class=\"config-element\">";
  pgStr += "              <p>WiFi server settings.</p>";
  pgStr += "              <form action=\"/restartNetwork\" method=\"POST\">";
  pgStr += "                <label for=\"channel\">WiFi Channel:</label>";
  pgStr += "                <select id=\"channel\" name=\"channel\">";
  pgStr += getChannelOptionList(channel);
  pgStr +="                 </select><br>";
  pgStr +="                 <label for=\"IPAddress\">IP Address:</label>";
  pgStr +="                 <select id=\"IPAddress\" name=\"IPAddress\">";
  pgStr += getIPAddressOptionList(addr);
  pgStr += "                </select><br>";
  pgStr += "                <p>Note: After you press 'Save and Restart Network' you will have to reconnect";
  pgStr += "                          to the newtwork access point.  Also, if you have changed the IP Address that the Server";
  pgStr += "                           will use, you will have to use the new IP Address to access this page.";
  pgStr += "                           Ensure that you have noted the new IP Address if you do change it.</p>";
  pgStr += "                <div class=\"buttonHolder\">";
  pgStr += "                  <input type=\"submit\" value=\"Save Settings and Restart Network\">";
  pgStr += "                </div>";
  pgStr += "              </form>";
  pgStr += "            </div>";  // config element
  pgStr += "          </div>";    // config pane
  pgStr += "        </section>";  // networking
  // Encoders section.
  pgStr += "        <section id=\"encoders\">";
  pgStr += "          <div class=\"config-pane\">";
  pgStr += "            <div class=\"config-element\">";
  pgStr += "              <p>Encoder settings.</p>";
  pgStr += "              <form action=\"/configureEncoders\" method=\"POST\">";
  pgStr += "                <label for=\"maxAzTics\">Azimuth Max Count:</label>";
  pgStr += "                <input type=\"text\" id=\"maxAzTics\" name=\"maxAzTics\" value=\"";
  pgStr +=                    maxAz;
  pgStr += "                \"><br>";
  pgStr += "                <label for=\"maxAlTics\">Altitude Max Count:</label>";
  pgStr += "                <input type=\"text\" id=\"maxAlTics\" name=\"maxAlTics\" value=\" ";
  pgStr +=                    maxAl;
  pgStr += "                \"><br>";
  pgStr += "                <p>Note: After you press 'Save Encoder Settings' the encoders will be reset using current position";
  pgStr += "                   as the new zero position.  This will break any alignment you have done in Sky Safari.</p>";
  pgStr += "                <div class=\"buttonHolder\">";
  pgStr += "                  <input type=\"submit\" value=\"Save Encoder Settings\">";
  pgStr += "                </div>";
  pgStr += "              </form>";
  pgStr += "            </div>";  // config element
  pgStr += "          </div>";    // config pane
  pgStr += "        </section>";  // encoders
  pgStr += "      </div>";        // config container
  pgStr += "    </body>";
  pgStr += "  </html>";

  return pgStr;
}

String getChannelOptionList(String channel){
  /* return HTML to create an option list for selecting the channel from 1 to 12 */
  String pgStr = "<option value=\"1\" ";
  if (channel == "1") {
      pgStr += "selected";
  }
  pgStr += ">1</option>";

  pgStr += "<option value=\"2\" ";
  if (channel == "2") {
    pgStr += "selected";
  }
  pgStr += ">2</option>";
    
  pgStr += "<option value=\"3\" ";
  if (channel == "3") {
    pgStr += "selected";
  }
  pgStr += ">3</option>";
    
  pgStr +="<option value=\"4\" ";
  if (channel == "4") {
    pgStr += "selected";
  }
  pgStr += ">4</option>";

  pgStr +="<option value=\"5\" ";
  if (channel == "5") {
    pgStr += "selected";
  }
  pgStr += ">5</option>";
  
  pgStr +="<option value=\"6\" ";
  if (channel == "6") {
    pgStr += "selected";
  }
  pgStr += ">6</option>";

  pgStr +="<option value=\"7\" ";
  if (channel =="7") {
    pgStr += "selected";
  }
  pgStr += ">7</option>";

  pgStr +="<option value=\"8\" ";
  if (channel == "8") {
    pgStr += "selected";
  }
  pgStr += ">8</option>";

  pgStr +="<option value=\"9\" ";
  if (channel == "9") {
    pgStr += "selected";
  }
  pgStr += ">9</option>";

  pgStr +="<option value=\"10\" ";
  if (channel == "10") {
    pgStr += "selected";
  }
  pgStr += ">10</option>";
    
  pgStr +="<option value=\"11\" ";
  if (channel == "11") {
    pgStr += "selected";
  }
  pgStr += ">11</option>";
    
  pgStr +="<option value=\"12\" ";
  if (channel == "12") {
    pgStr += "selected";
  }
  pgStr += ">12</option>";

  return pgStr;
}

String getIPAddressOptionList(String addr){
  /* return HTML to create an option list for selecting the IP Address from 1 to 12 */
  String pgStr ="<option value=\"1\" ";
  if (addr == "1") {
    pgStr += "selected";
  }
  pgStr += ">192.168.1.1</option>";
  
  pgStr += "<option value=\"2\" ";
  if (addr == "2") {
    pgStr += "selected";
  }
  pgStr += ">192.168.2.1</option>";
  
  pgStr += "<option value=\"3\" ";
  if (addr == "3") {
    pgStr += "selected";
  }
  pgStr += ">192.168.3.1</option>";
  
  pgStr +="<option value=\"4\" ";
  if (addr == "4") {
    pgStr += "selected";
  }
  pgStr += ">192.168.4.1</option>";
  
  pgStr +="<option value=\"5\" ";
  if (addr == "5") {
    pgStr += "selected";
  }
  pgStr += ">192.168.5.1</option>";

  pgStr +="<option value=\"6\" ";
  if (addr == "6") { 
    pgStr += "selected";
  }
  pgStr += ">192.168.6.1</option>";

  pgStr +="<option value=\"7\" ";
  if (addr == "7") {
    pgStr += "selected";
  }
  pgStr += ">192.168.7.1</option>";

  pgStr +="<option value=\"8\" ";
  if (addr == "8") {
    pgStr += "selected";
  } 
  pgStr += ">192.168.8.1</option>";

  pgStr +="<option value=\"9\" ";
  if (addr == "9") {
    pgStr += "selected";
  }
  pgStr += ">192.168.9.1</option>";

  pgStr +="<option value=\"10\" ";
  if (addr == "10") {
    pgStr += "selected";
  }
  pgStr += ">192.168.10.1</option>";
  
  pgStr +="<option value=\"11\" ";
  if (addr == "11") {
    pgStr += "selected";
  }
  pgStr += ">192.168.11.1</option>";
  
  pgStr +="<option value=\"12\" ";
  if (addr == "12") {
    pgStr += "selected";
  }
  pgStr += ">192.168.12.1</option>";

  return pgStr;
}

String getCSSSection(){
  String pgStr = "";
  
  pgStr += getColorScheme();

  // CSS reset
  pgStr += "* {";
  pgStr += "  margin: 0;";
  pgStr += "  padding: 0;";
  pgStr += "  box-sizing: border-box;";
  pgStr += "}";

  pgStr += "body {";
  pgStr += "  font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif;";
  pgStr += "  font-size: 1rem;";
  pgStr += "  min_height: 200vh;";
  pgStr += "}";

  pgStr += "#netwClass { background-color: var(--dark);}";
  pgStr += "$encClass { background-color: var(--dark);}";

  pgStr += ".config-pane {";
  pgStr += "  position: sticky;";
  pgStr += "  height: 100vh;";
  pgStr += "  background-color: var(--muted);";
  pgStr += "  color: var(--very-dark);";
  pgStr += "  width: 100%;";
  pgStr += "  text-align: left;";
  pgStr += "  top: 28vh;";
  pgStr += "}";

  pgStr += "label {";
  pgStr += "  display: inline-block;";
  pgStr += "  width: 35%;";
  pgStr += "  clear: left;";
  pgStr += "  text-align: right;";
  pgStr += "}";

  pgStr += "select {";
  pgStr += "    display: inline-block;";
  pgStr += "  }";

  pgStr += "input {";
  pgStr += "    display: inline-block;";
  pgStr += "  }";

  pgStr += "a:visited {";
  pgStr += "  color: var(--bright);";
  pgStr += "}";

  pgStr += "p {";
  pgStr += "  padding: 1.5rem;";
  pgStr += "}"; 

  pgStr += "option {";
  pgStr += "  font-family: inherit;";
  pgStr += "  font-size: 100%;";
  pgStr += "}";

  pgStr += "button, input, select, textarea {";
  pgStr += "  font-family: inherit;";
  pgStr += "  font-size: 100%;";
  pgStr += "  padding: 0.25rem;";
  pgStr += "  margin: 0.25rem;";
  pgStr += "  box-sizing: border-box;";
  pgStr += "}";

  pgStr += ".buttonHolder {";
  pgStr += "    text-align: center;";
  pgStr += "  }";

  pgStr += "#channel {";
  pgStr += "  width: 4rem;";
  pgStr += "}";

  pgStr += "#IPAddress {";
  pgStr += "  width: 16rem;";
  pgStr += "}"; 
  
  pgStr += "#maxAlTics {";
  pgStr += "  width: 6rem;";
  pgStr += "}";

  pgStr += "#maxAzTics {";
  pgStr += "  width: 6rem;";
  pgStr += "}";

  pgStr += ".page-header {";
  pgStr += "  position: fixed;";
  pgStr += "  width: 100%;";
  pgStr += "  height: 8vh;";
  pgStr += "  background-color: var(--very-dark);";
  pgStr += "  color: var(--muted);";
  pgStr += "  top: 0;";
  pgStr += "  left: 0;";
  pgStr += "  text-align: center;";
  pgStr += "  font-size: 2rem;";
  pgStr += "  z-index: 1;";
  pgStr += "}";

  pgStr += "nav {";
  pgStr += "  background-color: var(--very-dark);";
  pgStr += "  position: fixed;";
  pgStr += "  width: 100%;";
  pgStr += "  height: 8vh;";
  pgStr += "  top: 8vh;";
  pgStr += "  left: 0;";
  pgStr += "  font-size: 1.5rem;";
  pgStr += "  color: var(--bright);";
  pgStr += "  z-index: 1;";
  pgStr += "}";

  pgStr += ".config-container {";
  pgStr += "  position: absolute;";
  pgStr += "  top: 16vh;";
  pgStr += "}";

  pgStr += ".config-element {";
  pgStr += "  position: sticky;";
  pgStr += "  top: 16vh;";
  pgStr += "}";

  /* For hiding scroll bar */
  pgStr += "::-webkit-scrollbar{";
  pgStr += "  display: none;";
  pgStr += "}";
  
  pgStr += "html { scroll-behavior: smooth; }";

  return pgStr;
}

String getColorScheme(){
  // return the color scheme to be used on the page.
  String pgStr = "";
  pgStr += ":root {";
  pgStr += "  --white: #FFF;";
  pgStr += "  --very-bright: #EEE;";
  pgStr += "  --bright: #CCC;";
  pgStr += "  --muted: #999;";
  pgStr += "  --dark: #666;";
  pgStr += "  --very-dark: #333;";
  pgStr += "  --black: #000;";
  pgStr += "}";
  return pgStr;
}

} // namespace WebConfigServer
} // namespace grt