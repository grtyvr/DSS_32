#include "EventLoop.hpp"
#include "Event.hpp"

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

