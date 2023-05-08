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

//#define AS5048A_DEBUG

AS5048A::AS5048A(uint8_t cs){
  _settings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
  _cs = cs;
  pinMode(_cs, OUTPUT);
  _errorFlag = false;
}

void AS5048A::init(SPIClass* spi){
  _spi = spi;
}

void AS5048A::setSPIBus(SPIClass* spi){
  _spi = spi;
}

bool AS5048A::error(){
  return _errorFlag;
}

uint16_t AS5048A::getMagnitude(){
  uint16_t rawData = read(REG_MAG);
  // the bottom 14 bits are the magnitude
  return rawData &= 0x3FFF;
}

void AS5048A::update(){
  _curTics = read(REG_ANGLE);
  // the bottom 14 bits are the angle
  _curTics &= 0x3FFF;
  // apply filter to the raw data
//  _curTics = updateKalmanEstimate(_curTics);
  _curTics = updateExponentialEstimate(_curTics);

//  #ifdef AS5048A_DEBUG
//    Serial.print(" old value: ");
//    Serial.println(_curTics);
//    Serial.print(" ");
//  #endif
}

uint16_t AS5048A::getTics(){
#ifdef AS5048A_DEBUG
  Serial.print(" new value: ");
  Serial.println(_curTics);
#endif
  return _curTics;
}

uint16_t AS5048A::getMaxTics(){
  return _maxTics;
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
  return read(REG_AGC);
}

uint8_t AS5048A::getGain(){
  // the gain is in the bottom 8 bits
  return read(REG_AGC) & 0xFF;
}

float AS5048A::getSensorStdDev(uint8_t numSamples) {
  // Take numSamples of readings and return the sample standard deviation
  uint16_t samples[numSamples];
  uint32_t total = 0;
  #ifdef AS5048A_DEBUG
    unsigned long startTransactionTime;
    unsigned long stopTransactionTime;
  #endif
  // wake up the chip with a reset.
  reset();
  for (int i = 0; i < numSamples; i++){
    #ifdef AS5048A_DEBUG
      startTransactionTime = micros();
    #endif
    samples[i] = readAngle();
    #ifdef AS5048A_DEBUG
      stopTransactionTime = micros();
      Serial.print("Transaction Time: ");
      Serial.print(stopTransactionTime - startTransactionTime);
      Serial.print(" value: ");
      Serial.println(samples[i]);
    #endif
    total += samples[i];
  }
  uint16_t mean =  round(total / numSamples);
  #ifdef AS5048A_DEBUG
    Serial.print("Mean: ");
    Serial.println(mean);
  #endif
  float sqDiffTot = 0.0;
  for (int i = 0; i < numSamples; i++){
    sqDiffTot += pow(samples[i] - mean, 2);
  }
  return sqrt(sqDiffTot/numSamples);
}

uint16_t AS5048A::updateKalmanEstimate(uint16_t measurment) {

  _gain = _err_est/(_err_est + _err_meas);
  /* We have to be carefull of overflow and underflow... 
    * cases to treat:
   * last_estimate + _kalman_gain(mea - _last_estimate) > max
   * last_estimate + _kalman_gain(mea - _last_estimate) < 0
   * and we have to round the estimate to an uint16_t 
   */
  uint16_t _curr_est = round(_last_est + _gain * (measurment- _last_est));
  if ( _curr_est > _maxTics ) {
    _curr_est = _curr_est - _maxTics;
  } else if (_curr_est < 0 ) {
    _curr_est = _maxTics + _curr_est;
  }
  /* cases to treat:
   * We have to be carefull at the crossover from near max values and min values.
   * When |last_estimate - _current_estimate | is large ( bigger than half max say ) 
   * we have values that cross the zero point and we need to be a bit careful in 
   * calculating the difference between the reading and the estimate.  
  */
  uint16_t cur_delta = _last_est - _curr_est;
  // if you have a large negative value then the curr_est is large and last is small
  if (cur_delta < (-1 * _maxTics/2) ){
    cur_delta = cur_delta + _maxTics;
    // if you have a large positive value then cur_est is small and last is large
  } else if ( cur_delta > (_maxTics / 2)) {
    cur_delta = _maxTics - cur_delta;
  } else {
    // otherwise just use the absolute value of the difference since it is small
    // the cur_est and last are close numericaly.
    cur_delta = abs(cur_delta);
  }
  // update the estimated error and the current estimated value
  _err_est =  (1.0 - _gain)*_err_est + cur_delta*_q;
  _last_est =_curr_est;

  return _curr_est;
}

float AS5048A::getKalmanGain() {
  return _gain;
}

float AS5048A::getEstimateError() {
  return _err_est;
}

uint16_t AS5048A::updateExponentialEstimate(uint16_t newTics){
  /// use exponential smoothing to return the new reading
  /// since we might be crossing from 2^14 back to 0 we need to take care about sanitizing
  // our readings to make sure we have not wrapped around
  // One heuristic method is by making sure that our old value is not more than half
  // of the total range away from the new value
  // For example:
  //    1 ---> 16383 is just 3 tic
  //  We want to do something reasonable with that sort of thing
  //
  if (_curr_est - newTics > 8192){
    // we have wrapped from High to LOW
    // so move the new value out past the high end
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newTics += 16384;
    _curr_est = (_curr_est * (1 - _expSmoothFactor)) + (newTics * _expSmoothFactor);

  } else if (newTics - _curr_est > 8192){
    // here we have wrapped from Low to High
    // so move the new value to the low end ( may be negative but it still works)
    // calculate what the smoothed value would be as if the range was wider
    // and if that new value would move us out of range, then return
    // the wrapped value
    newTics -= 16384;
    _curr_est = (_curr_est * (1 - _expSmoothFactor)) + (newTics * _expSmoothFactor);
    // this could be negative.  If it is we have to wrap back to the top....
    if (_curr_est < 0){
      _curr_est += 16384;
    }
  } else {
    _curr_est = (_curr_est * (1 - _expSmoothFactor)) + (newTics * _expSmoothFactor);
  }
  return  ((uint16_t) round(_curr_est)) % 16384;
}

uint16_t AS5048A::read(uint16_t REGISTER){
  // Send a read command to the chip to read REGISTER
  // Note that the chip needs to have a second read
  // in order to get the data from the first command.
  uint16_t data;
  // send a read command to access the REGISTER
  // Set up the command we will send
  uint16_t command = CMD_READ | REGISTER;
  command |= getEvenParityBit(command) <<15;
  _spi->beginTransaction(_settings);
  // Drop cs low to enable the AS5048
  digitalWrite(_cs, LOW);
  data = _spi->transfer16(command);
  digitalWrite(_cs, HIGH);
  _spi->endTransaction();

  return data;
}

uint16_t AS5048A::readAngle(){
  // Send a read command to the chip to read the angle register
  // Note that the chip needs to have a second read
  // in order to get the data from the first command.
  uint16_t data = read(REG_ANGLE);
  data = read(CMD_NOP);

  if (!parityEven(data)){
    Serial.println("Odd Parity Returned.");
  }
  #ifdef AS5048A_DEBUG
    Serial.println();
    Serial.print("Sent Command: ");
    Serial.println(command, BIN);
    Serial.print(" To register: ");
    Serial.println(REGISTER, BIN);
    Serial.println(data);
  #endif
  if (data & 0x4000) {
    // if bit 15 is set then there was an error on the last command
    Serial.println("Error bit was set:");
  }
  return data & 0x3FFF;
}

void AS5048A::readErrorReg(){
  // if a previous command returns an error we call this procedure
  // to get the error register.  We only have one SPI command since
  // the error register was pre-populated by the previous command.
  uint16_t command;
  uint16_t data;
  command = CMD_READ | CLEAR_ERR;
  command |= getEvenParityBit(command) <<15;
  _spi->beginTransaction(_settings);
  digitalWrite(_cs, LOW);
  data = _spi->transfer16(command);
  _spi->endTransaction();
  // the bottom three bits now store error flags
  bool invalidCommand = (data >> 1) & 0x1;
  bool framingError = (data >> 2) & 0x1;
  if (data & 0x1){
    Serial.println("Parity error");    
  }
  if ((data >> 1) & 0x1) {
      Serial.println("Command invalid");
  }
  if ((data >> 2) & 0x1) {
      Serial.println("Framing error");
  }
}

void AS5048A::reset(){
  digitalWrite(_cs, LOW);
  digitalWrite(_cs, HIGH);
}
uint16_t AS5048A::getEvenParityBit(uint16_t value){
  //
  uint8_t count = 0;
  // loop through the 16 bits
  for (uint8_t i = 0; i < 16; i++) {
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

bool AS5048A::parityEven(uint16_t value){
    //
    bool retVal = true;
    uint16_t count = 0;
    // loop through the 16 bits
    for (uint8_t i = 0; i < 16; i++) {
        // if the rightmost bit is 1 increment our counter
        if (value & 0x1) {
            count++;
        }
        // shift off the rightmost bit
        value >>=1;
    }
    // all odd binaries end in 1
    if ((count & 0x1) == 1) {
        retVal = false;
    } else {
        retVal = true;
    }
    return retVal;
}