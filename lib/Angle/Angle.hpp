#pragma once
#include <Arduino.h>
//
// Angle
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

class Angle {
  public:
    // constructor with no arguments means that we have to set the angle / tics later
    Angle();
    // constructor that takes an unsigned 16 bit number and sets the radians as a positive rotation (CCW) from zero
    Angle(uint16_t tics);
    // constructor that takes a float representing the rotation in radians and set the tics as a positive rotation (CCW) from zero
    Angle(double radians);
    void setTics(uint16_t tics);
    void setRadians(double radians);
    uint16_t getTics();
    double getRadians();
    double x();
    double y();
  private:
    double _angleIncrement = (2* PI) / pow(2,14);  // The smallest angle we can represent with our encoder
    uint16_t _tics = 0;                            // The number of tics that best represents an angle in radians
    double _radians = 0.0;                         // The number of radians in tics based on our smalles angular increment
    double _x = 0.0;                               // x component of the angle on unit circle
    double _y = 0.0;                               // y component of the angle on unit circle
    uint8_t _nullZone = 0;
};
