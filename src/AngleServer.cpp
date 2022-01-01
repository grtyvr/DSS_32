//
// Angle Server
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

#include "AngleServer.hpp"
#include "Encoders.hpp"
#include "Network.hpp"

WiFiServer *server = NULL;
const int portNumber = 4001; 

namespace grt {
namespace AngleServer {

void initialize(){
    server = new WiFiServer(portNumber);
    server->begin();
    Serial.print("listening on port: ");
    Serial.println(portNumber);
}

void processClient(){
  // wait for a new client:
  WiFiClient client = server->available();
  
  if (client){
    #ifdef DEBUGGING
      Serial.println("New connection");
    #endif
    // Encoders::Position curPos = Encoders::getPosition();
    // int alAng = curPos.altitude;
    // int azAng = curPos.azimuth;
    char encoderResponse[20];
    while (client.connected()) {
      if (client.available() > 0) {
        char c = client.read();
        Serial.println(c);
        switch (c) {
          case 'H':
          {
            Encoders::Position maxTics = Encoders::getMaxTics();
            sprintf(encoderResponse, "+%i\t+%i\r\n",maxTics.altitude, maxTics.azimuth);
            client.print(encoderResponse);
            #ifdef DEBUGGING
              Serial.println(encoderResponse);
            #endif
            break;
          }
          case 'Z':
          {
            Encoders::Position maxTics = Encoders::getMaxTics();
            sprintf(encoderResponse, "Z +%i +%i\r\n",maxTics.altitude, maxTics.azimuth);
            client.print(encoderResponse);
            #ifdef DEBUGGING
              Serial.println(encoderResponse);
            #endif
            break;
          }
          case 'Q':
          {
            Encoders::Position curPos = Encoders::getPosition();
            sprintf(encoderResponse, "+%i\t+%i\r\n",curPos.altitude, curPos.azimuth);
            client.print(encoderResponse);
            Serial.println(encoderResponse);
            break;
          }
          default:
            Serial.print("unrecognized command: ");
            Serial.println(c);
            break;
          }
        // discard remaining bytes
        client.flush();
      }
    }
    client.stop();
  } // end this client
}

} // namespace AngleServer
} // namespace grt