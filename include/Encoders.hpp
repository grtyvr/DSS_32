#pragma once
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


namespace grt {
namespace Encoders{

/// The Encoder Values.
///
struct Position{
    int altitude;
    int azimuth;
};


/// Initialise this module
///
void initialize();

/// Update the encoders.
void update();

/// Get the current position.
///
Position getPosition();

Position getMaxTics();

void setMaxTics(Position maxTics);

}
}