#include "Event.hpp"
//
// Event Loop
// ---------------------------------------------------------------------------
// (c)2019 by Lucky Resistor. See LICENSE for details.
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
Event::Event()
    : _call(nullptr), _next(0){
}

Event::Event(Function call, uint32_t next)
    : _call(call), _next(next){
}

bool Event::isValid() const{
    return _call != nullptr;
}

bool Event::isReady(uint32_t currentTime) const{
    if (_next == currentTime) {
        return true;
    }
    const auto delta = _next - currentTime;
    if ((delta & static_cast<uint32_t>(0x80000000ul)) != 0){
        return true;
    }
    return false;
}
Event::Function Event::getCall() const{
    return _call;
}

void Event::clear(){
    _call = nullptr;
}