#include "EventLoop.hpp"
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

namespace EventLoop {
    const uint8_t cEventStackSize = 8;

    uint32_t gCurrentTime = 0;
    Event gEvents[cEventStackSize];

void initialize(){
    gCurrentTime = millis();    
}

void addEvent(const Event &newEvent){
    // find a free place in our event stack for our event
    // TODO: this should be replaced with a proper queue
    for (auto &event : gEvents) {
        if (!event.isValid()) {
            event = newEvent;
            break;
        }
    }
}

void addDelayedEvent(Function call, uint32_t delay){
    addEvent(Event(call, gCurrentTime + delay));
}

void processEvents(){
    for (auto &event : gEvents) {
        if (event.isValid() && event.isReady(gCurrentTime)) {
            const auto call = event.getCall();
            event.clear();
            call();
        }
    }
}

void loop(){
    const auto currentTime = millis();
    if (gCurrentTime != currentTime) {
        gCurrentTime = currentTime;
        processEvents();
    }

}
}

