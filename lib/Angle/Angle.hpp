#pragma once
#include <Arduino.h>

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
