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
#include <SPI.h>
#include "ArduinoHeaders.hpp"
#include "Encoders.hpp"
#include "AS5048.hpp"
#include "Loop.hpp"


namespace grt {
namespace Encoders{

float smoothingFactor = 0.1;
int alPin = 32;
int azPin = 5;

// pointer to store the SPI bus we will use
SPIClass *vspi = NULL;

AS5048A alEnc(alPin, 3);
AS5048A azEnc(azPin, 3);
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

/// Get the encoder readings.
///
Position getPosition(){
    Position curPos = {
        alEnc.getExpSmoothAngle(smoothingFactor),
        azEnc.getExpSmoothAngle(smoothingFactor)
    };
    return curPos;
}

}
}