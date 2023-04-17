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
  uint16_t rawData = read(AS5048_REG_MAG);
  // the bottom 14 bits are the magnitude
  return rawData &= 0x3FFF;
}

uint16_t AS5048A::getTics(){
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

  return rawData &= 0x3FFF;
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
  return read(AS5048_REG_AGC);
}

uint8_t AS5048A::getGain(){
  // the gain is in the bottom 8 bits
  return read(AS5048_REG_AGC) & 0xFF;
}

uint16_t AS5048A::read(uint16_t REGISTER){
  // read the sensors REG_DATA register.  Stores the angle.
  unsigned int data;
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
  // if bit 15 is set then there was an error on the last command
  if (data & 0x4000) {
    command = AS5048_READ_CMD | AS5048_CLEAR_ERR;
    command |= calcEvenParity(command);
    _spi->beginTransaction(_settings);
    digitalWrite(_cs, LOW);
    data = _spi->transfer16(command);
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);
    data = _spi->transfer16(command);
    digitalWrite(_cs, HIGH);
    _spi->endTransaction();
    switch(data & 0x7) {
      case 0x4:
        Serial.println("parity error on read.");
      break;
      
      case 0x2:
        Serial.println("Command Invalid.");
      break;

      case 0x1:
        Serial.println("Framing error.");
      break;
    }
    data = 0;  
  }
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