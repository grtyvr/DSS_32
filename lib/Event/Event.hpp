#pragma once
//
// Event
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
#include "Arduino.h"

class Event{
public:
    typedef void (*Function)();
public:
    /**
     * @brief Constructor.
     * create an invalid event.
     */
    Event();
    /**
     * @brief Constructor.
     * 
     * @param call The function that will be called when the event triggers
     * @param next The time that the event will trigger
     */
    Event(Function call, uint32_t next);
    // copy and assigment
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
public:
    bool isValid() const;
    bool isReady(uint32_t currentTime) const;
    Function getCall() const;
    void clear();
private:
    Function _call;
    uint32_t _next;
};