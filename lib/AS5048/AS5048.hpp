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

// AS5048A SPI Register Map
const uint16_t CMD_NOP = 0x0;       // no operation, dummy information.  Use this to get result of last command
const uint16_t REG_ERR = 0x1;       // Error Register.  To clear the register, access it.
                                    // bit 0, framing error, bit 1 Command invalid, bit 2 Parity Error.
const uint16_t PGM_CTL = 0x3;       // Programming control register.  Must enable before burning fuses.  Always 
                                    // verify after programing. bit 0: program enable - bit 3: burn -- bit 6: verify
const uint16_t OTP_0_HIGH = 0x16;   // Zero position high byte: bits 0..7  top 6 bits not used.
const uint16_t OTP_0_LOW = 0x17;    // Zero position lower 6 Least Significant Bits: bits 0..5, Top 8 bits not used.
const uint16_t REG_AGC = 0x3FFD;    // Diagnostic and Automatic Gain Control
                                    // Bits 0..7 AGC value 0=high, 255=low - Bit 8: OCF - Bit 9: COF - Bits 10..11 Comp Low..High
const uint16_t REG_MAG = 0x3FFE;    // Magnitude after ATAN calculation bits 0..13
const uint16_t REG_ANGLE = 0x3FFF;  // Angle after ATAN calculation and zero position correction if used - bits 0..13
const uint16_t CMD_READ = 0x4000;   // bit 15 = 1 for read operation.
const uint16_t CLEAR_ERR = 0x1;     // Clear error flag

constexpr std::uint16_t maskBottom14{0x3FFF};
constexpr std::uint16_t maskBottom8{0xFF};
constexpr std::uint16_t maskErrorFlag{0x4000};  // bit 15 is an error flag in a return value
constexpr std::uint16_t maskCompHigh{};

class AS5048A{
    private:
        uint16_t _curTics = 0;
        SPISettings _settings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
        SPIClass* _spi;
        bool _errorFlag;
        uint8_t _errorValue;
        uint8_t _cs;
        uint16_t _maxTics = 16384;
        float _err_meas = 3.0;
        float _err_est = 9.0;
        float _q = 0.01;
        float _curr_est = 0;
        float _last_est = 0;
        float _gain = 0;
        float _expSmoothFactor = 0.025; // weight new values by 2.5%
        uint8_t _maxTries = 5;

    public:
        /**
         * @brief Constructor.
         * 
         *  @param {uint8_t} cs The pin used for selecting the chip. 
         */
        AS5048A(uint8_t cs);

        /**
         * @brief Initialize the encoder.
         * 
         *  @param {SPIClass*} spi Bus that the chip will use for communications. 
         */
        void init(SPIClass* spi);

        /**
         * @brief Set SPI bus that the chip will use for communications.
         * 
         *  @param {SPIClass*} spi Bus that the chip will use for communications. 
         */
        void setSPIBus(SPIClass *spi);

        /**
         * getMagnitude
         * One of the diagnostic features of the AS5048 is that it can report the magnitude 
         * of the magnetic field
         * @return {uint16_t} magnitude.
         */
        uint16_t getMagnitude();
        
        /**
         * 
         * @brief Update the current value.
         * 
         * This is intended to be called from an update loop.  It will get a new reading
         * from the AS5048A, close the SPI bus and optionally apply a filter to the result
         */
        void update();
        
        /**
         * getTics
         * @return {uint16_t} angle as a value in the interval [0,2^14-1].  Rotation counter clockwise 
         * from the current zero position
         */
        uint16_t getTics();

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

        uint8_t getErrors();

        bool error();

        uint16_t getMaxTics();

        float getSensorStdDev(uint8_t samples);

    private:

        float getKalmanGain();
        float getEstimateError();
        uint16_t updateExponentialEstimate(uint16_t curTics);
        uint16_t getDiag();
        uint16_t getEvenParityBit(uint16_t value);
        bool parityEven(uint16_t value);
        uint16_t readAngle();
        void readErrorReg();
        void reset();
        uint16_t read(uint16_t registerAddress);
        // TODO: Implement the write command.  Part of the configure version?
        uint16_t write(uint16_t registerAddress, uint16_t data);
        uint16_t updateKalmanEstimate(uint16_t newVal);
};