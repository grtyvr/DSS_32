///////////////////////////////////////////////////////////////////////////////
//
//  Set up the functions for reading the sensors
//
// These defines are for the AS5048
// for example...
// http://ams.com/eng/Support/Demoboards/Position-Sensors/Rotary-Magnetic-Position-Sensors/AS5048A-Adapterboard
#include <Arduino.h>
#include "sensors.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Function definitions
//
///////////////////////////////////////////////////////////////////////////////
//
// pad the Tics value with leading zeros and return a string
//
// ToDo:  get rid of String!
//
String PadTic(unsigned int tic, String Sign){
  String paddedTic;
  if (tic < 10)
    paddedTic = "0000" + String(tic);
  else if ( tic < 100 )
    paddedTic = "000" + String(tic);
  else if ( tic < 1000 )
    paddedTic = "00" + String(tic);
  else if ( tic < 10000 )
    paddedTic = "0" + String(tic);
  else if ( tic < 100000 )
    paddedTic = "" + String(tic);
  paddedTic = Sign + paddedTic;
  return paddedTic;
}
///////////////////////////////////////////////////////////////////////////////
//
// Calculate Even parity of word
byte calcEvenParity(word value){
  byte count = 0;
  // loop through the 16 bits
  for (byte i = 0; i < 16; i++) {
    // if the rightmost bit is 1 increment our counter
    if (value & 0x1) {
      count++;
    }
    // shift off the rightmost bit
    value >>=1;
  }
  // all odd binaries end in 1
  return count & 0x1;
}


///////////////////////////////////////////////////////////////////////////////
//
// return the circular mean of the angles using the atan2 method
// takes a pointer to an array of angles
int circAve(int *angsToAve){
  double totX = 0;
  double aveX = 0;
  double totY = 0;
  double aveY = 0;
  double ang = 0;
  int retVal = 0;
  int k = (sizeof(angsToAve)/sizeof(angsToAve[0]));
  for ( int j = 0; j < k ; j++) {
//    yield();
   totX += cos((double) (ticsToAng(angsToAve[j]) * PI) / 180));
   totY += sin((double) (ticsToAng(angsToAve[j]) * PI) / 180));
  }
  aveX = totX / k;
  aveY = totY / k;
  if ( aveX == 0 && aveY == 0 ) {
    // just to be safe define a value where atan2 is undefined
    ang = 0;
  } else {
    ang = atan2(aveY , aveX);
    if (ang >= 0) {
      // if the returned value of angle is positive
      // it is a positive rotation (CCW) between 0 and 180 degress
      retVal = angToTics((ang / PI) * 180);
    } else {
      // convert the negative angle to a positive rotation from 0 degrees (CCW)
      retVal =  angToTics(((( 2 * PI ) + ang) / PI ) * 180);
    }
  }
  return retVal;
}
///////////////////////////////////////////////////////////////////////////////
//
// http://damienclarke.me/code/posts/writing-a-better-noise-reducing-analogread
//
///////////////////////////////////////////////////////////////////////////////
//
// return the exponentially smoothed value paying close attention to
// how we need to wrap the smoothing out around the transition from
// 2^14 back to 0
//
//
int expSmooth(int oldVal, int newVal float smoothingFactor){
  int retVal = oldVal;
  // do some bounds checking
  if (smoothingFactor > 1){
    smoothingFactor = 1
  } elseif (smoothingFactor <0 ){
    smoothingFactor = 0
  }
  // make sure we have not wrapped around
  // we do that by making sure that our old value is not more than half
  // of the total range away from the new value
  // For example:
  //    1 ---> 16383 is just 3 tic
  //  We want to do something reasonable with that sort of thing
  //
  if (oldVal - newVal > 8192){
    // we have wrapped from High to LOW
    // so move the new value out past the high end
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newVal += 16384;
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor)) % 16384;
  } elseif (newVal - oldVal > 8192){
    // here we have wrapped from low to high
    // so move the new value to the low end ( may be negative but it still works)
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newVal -= 16384;
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor));
    // this could be negative.  If it is we have to wrap back to the top....
    if (retVal < 0){
      retVal += 16384;
    }
  } else {
    retVal = (int) ((oldVal * smoothingFactor) + newVal * ( 1 - smoothingFactor));
  }
}
///////////////////////////////////////////////////////////////////////////////
//
// advancedCircularSmooth
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
int aCircSmth(int newAng, int axis, int curCircSmthVal, int *pastRead){
  static int alArrayPos;  // this is a hack to take into account we are calling this function on two different arrays
  static int azArrayPos;  //
  int curPos;
  double tmpFloat;
  int tmpInt;
  int numToAve = sizeof(pastRead)/sizeof(pastRead[axis]);
  int srtdAngles[numToAve];
  double srtdDeltas[numToAve];
  int srtdIndex[numToAve];
  int angsToAve[numToAve - 2 * numToDscd];
  // increment the couter based on the axis we are smoothing
  if ( axis == 1 ) {
    alArrayPos = ( alArrayPos + 1) % numToAve;
    curPos = alArrayPos;
  } else {
    azArrayPos = ( azArrayPos + 1) % numToAve;
    curPos = azArrayPos;
  }
  pastRead[curPos] = newAng;                    // put the new value into the array
  yield();
  for ( int j = 0; j < numToAve; j++ ) {
     srtdAngles[j] = pastRead[j];               // create our array for sorting
     srtdDeltas[j] = angSep(curCircSmthVal, pastRead[j]);
     srtdIndex[j] = j;                          // create our array of inexes to store sorted order
  }
  // do an insertion sort on the deltas array and apply the transformations to the index array
  // and the sorted Angles array at the same time
  for ( int k = 1 ; k < numToAve; k++){
    // for all but the first element look at the remaining elements till a smaller element is found
    j = k;
    yield();
    while (j > 0 && srtdDeltas[j-1] > srtdDeltas[j]){
      tmpFloat = srtdDeltas[j];
      srtdDeltas[j] = srtdDeltas[j-1];
      srtdDeltas[j-1] = tmpFloat;
      tmpFloat = srtdAngles[j];
      srtdAngles[j] = srtdAngles[j-1];
      srtdAngles[j-1] = tmpFloat;
      tmpInt = srtdIndex[j];
      srtdIndex[j] = srtdIndex[j-1];
      srtdIndex[j-1] = tmpInt;
      j -= 1;
      yield();
    }
  }
  // now create a smaller array discarding the top and bottom discardNumber values.
  for ( j = numToDscd; j < numToAve - numToDscd; j++ ){
    angsToAve[j - numToDscd] = srtdAngles[j - numToDscd];
  }
  return circAve(angsToAve);
}
///////////////////////////////////////////////////////////////////////////////
//
// Return the minimal angular separation for two angeles.
// Returns between 0 and 180 for any two input values
int angSep(int angOne, int angTwo){
  int retVal = abs(angOne - angTwo);
  if ( retVal > 8192 ) {
   retVal = 16384 - retVal;
  }
  return retVal;
}
///////////////////////////////////////////////////////////////////////////////
//
// convert an angle to tics
int angToTics( double ang){
  unsigned int retVal = 0;
  retVal = (unsigned int) (((ang + (increment / 2)) / 360 ) * 16384);
  if ( retVal > 16383 ) {
    retVal = retVal - 16384;
  }
  return retVal;
}
///////////////////////////////////////////////////////////////////////////////
//
// convert tics to angle
double ticsToAng( int tics){
  double retVal;
  retVal = (double)tics * increment;
  return retVal;
}
//  end of AS5048 calls
//
///////////////////////////////////////////////////////////////////////////////
