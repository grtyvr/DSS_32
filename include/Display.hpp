#pragma once
//
// Display
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
#include <U8g2lib.h>
#include <SPI.h>
#include "ArduinoHeaders.hpp"
#include "Encoders.hpp"

namespace grt {
namespace Display {

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2; 


    /// The Display Modes.
    ///
    enum DisplayMode : uint8_t {
        Test = 0,
        Run,
        None = 0xffu
    };

    // Initialize the display
    // 
    void initialize();

    // Set the mode of the display
    //
    void setMode();

    // Get the mode of the display
    //
    void getMode();

    // update the display with new data
    //
    void update();

} // namespace Display
} // namespace lr
