//
// Display
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

#include "Display.hpp"
#include "Encoders.hpp"
#include "Network.hpp"

namespace grt {
namespace Display {
    
// The type of display.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2;


// Initialize the display
// 
void initialize(){
    u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R2);
    u8g2->begin();
}

// Set the mode of the display
//
void setMode(){

}

// Get the mode of the display
//
void getMode(){

}

// update the display with new data
//
void update(){
  Encoders::Position curPos = Encoders::getPosition();
  int alAng = curPos.altitude;
  int azAng = curPos.azimuth;

  char tmp_string[12];
  u8g2->clearBuffer();
  u8g2->drawFrame(0,0,128,42);
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setDrawColor(1);
  u8g2->drawStr(3,17, "Al: ");
  u8g2->drawStr(3,37, "Az: ");
  uint8_t totNumWidth = u8g2->getStrWidth("00000");
  uint8_t xOffset = 125 - totNumWidth;
  itoa(alAng, tmp_string, 10);
  uint8_t numWidth = u8g2->getStrWidth(tmp_string);
  uint8_t xPos = xOffset + totNumWidth - numWidth;
  u8g2->drawStr(xPos,17,tmp_string);
  itoa(azAng, tmp_string, 10);
  numWidth = u8g2->getStrWidth(tmp_string);
  xPos = xOffset + totNumWidth - numWidth;
  u8g2->drawStr(xPos,37,tmp_string);
  u8g2->setFont(u8g2_font_profont12_mr);
  IPAddress myIP = WiFi.softAPIP();
  String ipAddr = myIP.toString();
  char buf[ipAddr.length() + 1];
  ipAddr.toCharArray(buf, ipAddr.length() + 1);
  u8g2->drawStr(3,61,"IP: ");
  u8g2->drawStr(25,61, buf);
  u8g2->sendBuffer();
}

} // namespace Display
} // namespace lr