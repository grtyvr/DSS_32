//
// Encoders
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
#include "ArduinoHeaders.hpp"
#include "Encoders.hpp"
#include "AS5048.hpp"
#include "Loop.hpp"
#include "Preferences.h"
#include "SPI.h"

namespace grt {
namespace Encoders{

int alPin = 25;
int azPin = 4;

AS5048A alEnc(alPin);
AS5048A azEnc(azPin);

SPIClass* vspi = NULL;

//
///////////////////////////////////////////////////////////////////////////////

/// Initialise this module
///
void initialize(){
  // Initialize SPI Bus.  VSPI defines the standard pinout as:
  // SCK = 18, CIPO = 19, COPI = 23, PS = 5
  vspi = new SPIClass(VSPI);
  // Wake up the bus
  vspi->begin();
  alEnc.setSPIBus(vspi);
  azEnc.setSPIBus(vspi);
}

/// @brief  Update the encoders
void update(){
    alEnc.update();
    azEnc.update();
}

/// Get the encoder readings.
///
Position getPosition(){
    Position curPos = {
        alEnc.getTics(),
        azEnc.getTics()
    };
    return curPos;
}

Position getMaxTics(){
    Position maxTics = {
        alEnc.getMaxTics(),
        azEnc.getMaxTics()
    };
    return maxTics;
}
void setMaxTics(Position maxTics){
    // empty in the case of AS5048 encoders
}
}
}