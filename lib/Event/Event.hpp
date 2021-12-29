#pragma once

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