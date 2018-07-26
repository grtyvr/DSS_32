///////////////////////////////////////////////////////////////////////////////
//
//  Set up the functions for reading the sensors
//
// These defines are for the AS5048
// for example...
// http://ams.com/eng/Support/Demoboards/Position-Sensors/Rotary-Magnetic-Position-Sensors/AS5048A-Adapterboard
#include <SPI.h>
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
byte calcEvenParity(word value) {
  byte count = 0;
  byte i;
  // loop through the 16 bits
  for (i = 0; i < 16; i++) {
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
// Read the sensor REG_DATA register
//
// ToDo:  This is a 32 bit chip.  Why!? am i splitting into high and low byte
//         rewrite this to use transfer16()
unsigned int readSensor(int cs){
  unsigned int data;
  unsigned int cmdHbyte;
  unsigned int cmdLbyte;
  unsigned int datHbyte;
  unsigned int datLbyte;
  pinMode(cs, OUTPUT);
  word command = AS5048_CMD_READ | AS5048_REG_DATA;                        // Set up the command we will send
  command |= calcEvenParity(command) <<15;                            // assign the parity bit
  cmdHbyte = highByte(command);                                   // split it into bytes
  cmdLbyte = lowByte(command);                                     //
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
  digitalWrite(cs, LOW);                                             // Drop ssl to enable the AS5048's
  datHbyte = vspi->transfer(cmdHbyte);                         // send the initial read command
  datLbyte = vspi->transfer(cmdLbyte);
  digitalWrite(cs, HIGH);                                            // disable the AS5048's
  digitalWrite(cs, LOW);                                             // Drop ssl to enable the AS5048's
  datHbyte = vspi->transfer(cmdHbyte);                         // send the initial read command
  datLbyte = vspi->transfer(cmdLbyte);
  digitalWrite(cs, HIGH);                                            // disable the AS5048's
  vspi->endTransaction();
  data = datHbyte;                                               // Store the high byte in my 16 bit varriable
  data = data << 8;                                                   // shift left 8 bits
  data = data | datLbyte;                                         // tack on the low byte
  #ifdef DEBUGGING
    Serial.println();
    Serial.print("Sent Command: ");
    Serial.println(command, BIN);
    Serial.print("To register: ");
    Serial.println(AS5048_REG_DATA, BIN);
    Serial.println(data);
  #endif
  return data;
}
///////////////////////////////////////////////////////////////////////////////
//
// This just trims the bottom 14 bits off of a sensor read
unsigned int readTic(int cs){
  unsigned int rawData;
  unsigned int realData;
  rawData = readSensor(cs);
  realData = rawData & 0x3fff;
  return realData;
}
///////////////////////////////////////////////////////////////////////////////
//
// Return the minimal angular separation for two angeles.  Returns between 0 and 180 for any two input values
double angSep(double angOne, double angTwo){
  double retVal = 0;
  retVal = fabs(angOne - angTwo);
  if ( retVal > 180 ) {
   retVal = 360 - retVal;
  }
  return retVal;

}
///////////////////////////////////////////////////////////////////////////////
//
// return the circular mean of the angles using the atan2 method
// takes a pointer to an array of angles
double circAve( double *angsToAve){
  int j;
  int k;
  double totX = 0;
  double aveX = 0;
  double totY = 0;
  double aveY = 0;
  double ang = 0;
  double retVal = 0;
  k = numToAve - ( 2 * numToDscd );
  for ( j = 0; j < k ; j++) {
    yield();
   totX += cos((double) ((angsToAve[j] * PI) / 180));
   totY += sin((double) ((angsToAve[j] * PI) / 180));
  }
  aveX = totX / k;
  aveY = totY / k;
  if ( aveX == 0 && aveY == 0 ) {
    ang = 0;                                 // just to be safe define a value where atan2 is undefined
  } else {
    ang = atan2(aveY , aveX);
    if (ang >= 0) {
      // if the returned value of angle is positive it is a positive rotation (CCW) between 0 and 180 degress
      retVal = (double) ((ang / PI) * 180);
    } else {
      // convert the negative angle to a positive rotation from 0 degrees (CCW)
      retVal =  (double) ((( 2 * PI ) + ang) / PI ) * 180;
    }
  }
  return retVal;
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
double aCircSmth(double newAng, int axis, double curCircSmthVal, double *pastRead) {
  int j, k;
  static int curPos1;  // this is a hack to take into account we are calling this function on two different arrays
  static int curPos2;  //
  int curPos;
  double tmpFloat;
  int tmpInt;
  double srtdAngles[numToAve];
  double srtdDeltas[numToAve];
  int srtdIndex[numToAve];
  double angsToAve[numToAve - 2 * numToDscd];
  // increment the couter based on the axis we are smoothing
  if ( axis == 1 ) {
    curPos1 = ( curPos1 + 1) % numToAve;
    curPos = curPos1;
  } else {
    curPos2 = ( curPos2 + 1) % numToAve;
    curPos = curPos2;
  }
  pastRead[curPos] = newAng;           // put the new value into the array
  yield();
  for ( j = 0; j < numToAve; j++ ) {
     srtdAngles[j] = pastRead[j];               // create our array for sorting
     srtdDeltas[j] = angularSeparation(curCircSmthVal, pastRead[j]);
     srtdIndex[j] = j;                                    // create our array of inexes to store sorted order
  }
  // do an insertion sort on the deltas array and apply the transformations to the index array
  // and the sorted Angles array at the same time
  for ( k = 1 ; k < numToAve; k++){
    // for all but the first element look at the remaining elements till a smaller element is found
    j = k;
    yield();
    while (j > 0 && srtdDeltas[j-1] > srtdDeltas[j]) {
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
  for ( j = numToDscd; j < numToAve - numToDscd; j++ ) {
    angsToAve[j - numToDscd] = srtdAngles[j - numToDscd];
  }
  return (double) circAve(angsToAve);
}
///////////////////////////////////////////////////////////////////////////////
//
// convert an angle to tics
unsigned int angToTics( double ang){
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
double ticsToAng( unsigned int tics) {
  double retVal;
  retVal = tics * increment;
  return retVal;
}
//  end of AS5048 calls
//
///////////////////////////////////////////////////////////////////////////////
