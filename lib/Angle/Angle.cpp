#include "Angle.hpp"
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

// constructor with no arguments means that we have to set the angle / tics later
Angle::Angle(){
    _tics = 0;
    _radians = 0.0;
    _x = 0.0;
    _y=0.0;
}
// constructor that takes an unsigned 16 bit number and sets the radians as a positive rotation (CCW) from zero
Angle::Angle(uint16_t tics){
    _tics = tics;
    _radians = _angleIncrement * _tics;
    _x = cos(_radians);
    _y = sin(_radians);
}
// constructor that takes a float representing the rotation in radians and set the tics as a positive rotation (CCW) from zero
Angle::Angle(double radians){
    _radians = radians;
    // first normalize the value so that it's absolute value is in [0, 2*PI)
    // do that by counting the number of full rotations in the input value (multiples of 2PI) using the floor function
    if (_radians >= 2*PI){
    // subtract the number of whole clockwise rotations over 1
    _radians = _radians - floor(_radians/(2*PI))*2*PI;
    } else if (_radians < 0) { 
    // our value is a clockwise rotation from zero.
    // so in this case add the number of whole rotations clockwise
    _radians = _radians + floor(abs(_radians)/(2*PI))*2*PI;
    // then normalize this to a positive rotation counter clockwise
    _radians = 2*PI + _radians;
    }
    // now we have a positive angle that is in the interval[0,2*PI)
    // so digitize that value to the nearest angular increment 
    _tics = (uint16_t) round(_radians / _angleIncrement);
    _x = cos(_radians);
    _y = sin(_radians);
}

void Angle::setTics(uint16_t tics){
    _tics = tics;
    _radians = _angleIncrement * _tics;
    _x = cos(_radians);
    _y = sin(_radians);
}
void Angle::setRadians(double radians){
    _radians = radians;
    // first normalize the value so that it's absolute value is in [0, 2*PI)
    // do that by counting the number of full rotations in the input value (multiples of 2PI) using the floor function
    if (_radians >= 2*PI){
    // subtract the number of whole clockwise rotations over 1
    _radians = _radians - floor(_radians/(2*PI))*2*PI;
    } else if (_radians < 0) { 
    // our value is a clockwise rotation from zero.
    // so in this case add the number of whole rotations clockwise
    _radians = _radians + floor(abs(_radians)/(2*PI))*2*PI;
    // then normalize this to a positive rotation counter clockwise
    _radians = 2*PI + _radians;
    }
    // now we have a positive angle that is in the interval[0,2*PI)
    // so digitize that value to the nearest angular increment 
    _tics = (uint16_t) round(_radians / _angleIncrement);
    _x = cos(_radians);
    _y = sin(_radians);
}
uint16_t Angle::getTics(){
    return _tics;
}
double Angle::getRadians(){
    return _radians;
}
double Angle::x(){
    return _x;
}
double Angle::y(){
    return _y;
}
