//
// Network
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

#include "Network.hpp"
#include <esp_wifi.h>

const char *apssid = "DSC_AP";
const char *appass = "";
const int defaultChannel = 10;

namespace grt {
namespace Network {

void initialize(){
    // Set up networking
    Serial.println("Setting up WiFi Access Point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apssid, appass, defaultChannel);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void printWiFiStatus(){
  // print the SSID your are broadcasting
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

} // namespace Network
} // namespace grt