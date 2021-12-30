//
// AS5048
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

#include <Arduino.h>
#include "Angle.hpp"

#include "AS5048.hpp"

// #define AS5048A_DEBUG

// AS5048A SPI Register Map
const uint16_t AS5048_CMD_NOP = 0x0;      // no operation, dummy information.  Use this to get result of last command
const uint16_t AS5048_REG_ERR = 0x1;      // Error Register.  To clear the register, access it.
                                          // bit 0, framing error, bit 1 Command invalid, bit 2 Parity Error.
const uint16_t AS5048_PRM_CTL = 0x3;      // Programming control register.  Must enable before burning fuses.  Always 
                                          // verify after programing. bit 0: program enable - bit 3: burn -- bit 6: verify
const uint16_t AS5048_OTP_0_HIGH = 0x16;  // Zero position high byte: bits 0..7  top 6 bits not used.
const uint16_t AS5048_OTP_0_LOW = 0x17;   // Zero position lower 6 Least Significant Bits: bits 0..5, Top 8 bits not used.
const uint16_t AS5048_REG_AGC = 0x3FFD;   // Diagnostic and Automatic Gain Control
                                          // Bits 0..7 AGC value 0=high, 255=low - Bit 8: OCF - Bit 9: COF - Bits 10..11 Comp Low..High
const uint16_t AS5048_REG_MAG = 0x3FFE;   // Magnitude after ATAN calculation bits 0..13
const uint16_t AS5048_REG_ANGLE = 0x3FFF; // Angle after ATAN calculation and zero position correction if used - bits 0..13

const uint16_t AS5048_READ_CMD = 0x4000;  // bit 15 = 1 for read operation.   

AS5048A::AS5048A(uint8_t arg_cs, uint8_t nullZone){
  _settings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
  _cs = arg_cs;
  _nullZone = nullZone;
  pinMode(_cs, OUTPUT);
  _errorFlag = false;
  _angle = 0.0;                           // initialize to zero degrees
}

void AS5048A::setSPIBus(SPIClass* spi){
  _spi = spi;
}

bool AS5048A::error(){
  return _errorFlag;
}

uint16_t AS5048A::getMagnitude(){
  uint16_t rawData = read(AS5048_REG_MAG);
  // the bottom 14 bits are the magnitude
  return rawData &= 0x3FFF;
}

uint16_t AS5048A::getAngle(){
  uint16_t rawData = read(AS5048_REG_ANGLE);
  // the bottom 14 bits are the angle
  #ifdef AS5048A_DEBUG
    rawData &= 0x3FFF;
    Serial.print(" null zone: ");
    Serial.print(_nullZone);
    Serial.print(" ");
    Serial.print(" old angle: ");
    Serial.print(_angle);
    Serial.print(" ");
  #endif
  if (abs(rawData - _angle) > _nullZone){
    _angle = rawData; 
    return rawData &= 0x3FFF;
  } else {
    return _angle;
  }
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
uint16_t AS5048A::getExpSmoothAngle(float smoothingFactor){
  /// use exponential smoothing to return the new reading
  /// since we might be crossing from 2^14 back to 0 we need to take care about normalizing our readings
  uint16_t newAngle = read(AS5048_REG_ANGLE);
  // the bottom 14 bits are the angle
   newAngle &= 0x3FFF;
  // make sure we have not wrapped around
  // we do that by making sure that our old value is not more than half
  // of the total range away from the new value
  // For example:
  //    1 ---> 16383 is just 3 tic
  //  We want to do something reasonable with that sort of thing
  //
  if (_angle - newAngle > 8192){
    // we have wrapped from High to LOW
    // so move the new value out past the high end
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newAngle += 16384;
    _angle = (_angle * (1 - smoothingFactor)) + (newAngle * smoothingFactor);
  } else if (newAngle - _angle > 8192){
    // here we have wrapped from Low to High
    // so move the new value to the low end ( may be negative but it still works)
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newAngle -= 16384;
    _angle = (_angle * (1 - smoothingFactor)) + (newAngle * smoothingFactor);
    // this could be negative.  If it is we have to wrap back to the top....
    if (_angle < 0){
      _angle += 16384;
    }
  } else {
    _angle = (_angle * (1 - smoothingFactor)) + (newAngle * smoothingFactor);
  }
  return  ((uint16_t) round(_angle)) % 16384;
}

uint16_t AS5048A::getMeanAngle(int numSamples){
  uint16_t retVal;
  float meanX = 0.0;
  float meanY = 0.0;
  Angle sample;
  /// take a number of samples and return the circular mean value
  /// since we might have a situation where we are sampling at the transition from
  /// 2^14 - n to m for small integer n and small integer m, have to be carefull to normalize our data
  for (int i=0; i < numSamples; i++){
    // take a sample and compute the x and y coordinate of the sample as if it on the unit circle
    sample.setTics(read(AS5048_REG_ANGLE));
    meanX += sample.x();
    meanY += sample.y();
  }
  // Take the average X and average Y values
  meanX = meanX/numSamples;
  meanY = meanY/numSamples;
  if ( (meanX == 0.0) & (meanY ==0.0) ){
    // pathalogical case of both X and Y being equal to zero as floats!
    retVal = 0;
  } else {
    Angle angle(atan2(meanY, meanX));
    if (abs(_angle - angle.getTics()) > _nullZone){
      retVal = angle.getTics();
    } else {
      retVal = _angle;
    }
    _angle = angle.getTics();
  }
  return retVal; 
}

void AS5048A::printDiagnostics(){
  uint16_t rawData = getDiag();
  Serial.print("AGC Value: ");
  // bottom 8 bits are the AGC value
  Serial.println(rawData & 0xFF);
  // bit 9 is OCF
  Serial.print("Offset Compensation Finished: ");
  Serial.print((rawData >> 8) & 0x1);
  // bit 10 is COF
  Serial.print(" - Cordic OverFlow: ");
  Serial.println((rawData >> 9) & 0x1);
    // bit 11 is Comp Low
  Serial.print("Comp Low: ");
  Serial.print((rawData >> 10) & 0x1);
  // bit 12 is Comp High
  Serial.print(" - Comp High: ");
  Serial.println((rawData >> 11) & 0x1);
}

uint16_t AS5048A::getDiag(){
  return read(AS5048_REG_AGC);
}

uint8_t AS5048A::getGain(){
  // the gain is in the bottom 8 bits
  return read(AS5048_REG_AGC) & 0xFF;
}

uint8_t AS5048A::getErrors(){
  _errorFlag = false;
  // To get the value of the error flags we have to send a special read command
  // The command clears the ERROR FLAG which is contained in every READ frame.
  // read the error register of the last command
  uint8_t retVal;
  // Set up the command we will send
  uint16_t cmdClearErrorFlag = 0x4001;  // 0b0100000000000001
  _spi->beginTransaction(_settings);
  // Drop cs low to enable the AS5048
  digitalWrite(_cs, LOW);
  retVal = _spi->transfer16(cmdClearErrorFlag);
  digitalWrite(_cs, HIGH);
  delayMicroseconds(100);
  digitalWrite(_cs, LOW);
  // the next two commands are NOP commands 
  // the first one is to trigger the return of the Error Register contents
  // and will still have the Error Flag Set.
  retVal = _spi->transfer16(0x0);
  digitalWrite(_cs, HIGH);
  delayMicroseconds(100);
  digitalWrite(_cs, LOW);
  // The second one will trigger the clearing of the Error Flag and we do not
  // get any usefull information back with that command
  _spi->transfer16(0x0);
  digitalWrite(_cs, HIGH);
  _spi->endTransaction();

  return retVal;
}


uint16_t AS5048A::read(uint16_t REGISTER){
  // read the sensors REG_DATA register.  Stores the angle.
  unsigned int data;
  pinMode(_cs, OUTPUT);
  // Set up the command we will send
  word command = AS5048_READ_CMD | REGISTER;
  command |= calcEvenParity(command) <<15;
  _spi->beginTransaction(_settings);
  // Drop cs low to enable the AS5048
  digitalWrite(_cs, LOW);
  data = _spi->transfer16(command);
  digitalWrite(_cs, HIGH);
  digitalWrite(_cs, LOW);
  // you have to poll the chip twice.  Data from previous command comes
  // back on the next SPI transfer
  data = _spi->transfer16(command);
  digitalWrite(_cs, HIGH);
  _spi->endTransaction();
  #ifdef AS5048A_DEBUG
    Serial.println();
    Serial.print("Sent Command: ");
    Serial.println(command, BIN);
    Serial.print(" To register: ");
    Serial.println(REGISTER, BIN);
    Serial.println(data);
  #endif
  return data;
}

byte AS5048A::calcEvenParity(uint16_t value){
  //
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