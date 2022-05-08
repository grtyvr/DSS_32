#pragma once
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

#include <SPI.h>

class AS5048A{
    private:
        SPISettings _settings;
        SPIClass* _spi;
        bool _errorFlag;
        uint8_t _errorValue;
        uint8_t _cs;
        float _angle;  // stores the last angle read
        uint8_t _nullZone = 0;
        uint16_t _maxTics = 16384;

    public:
        /**
         * @brief Constructor.
         * 
         *  @param {uint8_t} arg_cs The pin used for selecting the chip. 
         *  @param {uint8_t} nullZone The number of ticks to use as a null zone.
         */
        AS5048A(uint8_t arg_cs, uint8_t nullZone = 3);
        
        /**
         * getMagnitude
         * One of the diagnostic features of the AS5048 is that it can report the magnitude 
         * of the magnetic field
         * @return {uint16_t} magnitude.
         */
        void setSPIBus(SPIClass *spi);
        
        uint16_t getMagnitude();
        
        /**
         * getAngle
         * @return {uint16_t} angle as a value in the interval [0,2^14-1].  Rotation counter clockwise 
         * from the current zero position
         */
        uint16_t getAngle();
        
        /**
         * getExpSmoothAngle
         * @param {float} smoothingFactor
         * @return {uint16_t} angle as a value in the interval [0,2^14-1].  Rotation counter clockwise
         * from the current zero position.  This value is caclulated from the last angle read and the 
         * current angle read by the following formula
         * new_angle = old_angle*(1 - smoothingFactor) + new_angle*smoothingFactor
         */
        uint16_t getExpSmoothAngle(float smoothingFactor);
        
        /**
         * getMeanAngle
         * @param {int} numSamples
         * @return {uint16_t} mean value from a sample of numSample angle readings as a value in the 
         * interval [0,2^14-1].  Rotation counter clockwise from the current zero position.  
         * This value is the circular mean of the angles read.
         */
        uint16_t getMeanAngle(int numSamples);
        
        /**
         * printDiagnostics
         * Print diagnostic information to the serial port 
         */
        void printDiagnostics();
        
        /**
         * getGain
         * @return {uint8_t} Return the current Gain register value. The gain is stored in the 
         * first 8 bits of the Diagnostics + Automatic Gain Control register 0x3FFD
         */
        uint8_t getGain();

        bool error();

        uint16_t getMaxTics();

    private:
        uint16_t getDiag();
        uint8_t calcEvenParity(uint16_t value);
        uint16_t read(uint16_t registerAddress);
        uint16_t write(uint16_t registerAddress, uint16_t data);
};