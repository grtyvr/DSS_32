///////////////////////////////////////////////////////////////////////////////
//
//  Set up the functions for reading the sensors
//
// These defines are for the AS5048
// for example...
// http://ams.com/eng/Support/Demoboards/Position-Sensors/Rotary-Magnetic-Position-Sensors/AS5048A-Adapterboard
#include <Arduino.h>
#ifndef SENSORS_H
#define SENSORS_H

#define AS5048_CMD_READ 0x4000
#define AS5048_REG_AGC 0x3FFD
#define AS5048_REG_MAG 0x3FFE
#define AS5048_REG_DATA 0x3FFF
#define AS5048_REG_ERR 0x1
#define AS5048_CMD_NOP 0x0

const int numToAve = 20;
const int numToDscd = 2;
const double increment = (double) 360 / 16384;  // this is the smallest angular increment that is reportable by the sensors




///////////////////////////////////////////////////////////////////////////////
//
//  Function declarations
//
///////////////////////////////////////////////////////////////////////////////
//
// pad the Tics value with leading zeros and return a string
String padTic(unsigned int tic, String Sign);
///////////////////////////////////////////////////////////////////////////////
//
// Calculate Even parity of word
byte calcEvenParity(word value);

///////////////////////////////////////////////////////////////////////////////
//
// Read the sensor REG_DATA register
unsigned int readSensor(int cs);
///////////////////////////////////////////////////////////////////////////////
//
// This just trims the bottom 14 bits off of a sensor read
unsigned int readTic(int cs);
///////////////////////////////////////////////////////////////////////////////
//
// Return the minimal angular separation for two angeles.  Returns between 0 and 180 for any two input values
double angSep(double angOne, double angTwo);
///////////////////////////////////////////////////////////////////////////////
//
// return the circular mean of the angles using the atan2 method
// takes a pointer to an array of angles
double circAve( double *angsToAve);
///////////////////////////////////////////////////////////////////////////////
//
// aCircSmth
// take a new sensor reading, the current advancedCircularSmooth value and an array of past sensor readings
// return a good estimate of the most likely value that the sensor should read by doing the following
//
//   insert the new sensor reading into the oldest position in our array of past sensor readings
//   create a copy of the array for sorting and averaging
//   create an array of indexes that will represent the order of the original indicies in the sorted array
//   for each value in our past sensor values array
//     calculate the distance from the mean and store that value in a new array of (distance from mean)
//   do an insertion sort on this mean distance array and record the transpositions in the index array
//   throw out the bottom 10% and top 10% of values
//   use that array to find the new angular mean.
double aCircSmth(double newAng, int axis, double curCircSmthVal, double *pastRead);
///////////////////////////////////////////////////////////////////////////////
//
// convert an angle to tics
unsigned int angToTics( double angle);
///////////////////////////////////////////////////////////////////////////////
//
// convert tics to angle
double ticsToAng ( unsigned int tics);

#endif /* SENSORS_H */
