#pragma once
//
// OpticalEncoder
// ---------------------------------------------------------------------------
// (c)2023 by GRTYVR. See LICENSE for details.
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

#include "NewEncoder.h"
#include "Encoder.hpp"

namespace grt {
/**
 * OpticalEncoder Class
 * 
 * This is a hardware dependent class that makes the following assumptions about the underlying hardware:
 *  - the NewEncoder library is able to read the signals from the encoder and process them.  This will be true for a wide
 * range of hardware devices.  The NewEncoder library takes signals from quadrature encoders.
 *  - the encoders are  
*/
class OpticalEncoder: public NewEncoder{
    public:
        OpticalEncoder() : NewEncoder(){

        }
        OpticalEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initialValue = 0, uint8_t type = FULL_PULSE) :
            NewEncoder(aPin, bPin, minValue, maxValue, initialValue, type) {
        }
        virtual ~OpticalEncoder() {
        }
    protected:
        virtual void updateValue(uint8_t updatedState);  
};

void ESP_ISR OpticalEncoder::updateValue(uint8_t updatedState) {
    if ((updatedState & DELTA_MASK) == INCREMENT_DELTA){
        liveState.currentClick = UpClick;
        liveState.currentValue++;
        if (liveState.currentValue > _maxValue) {
            liveState.currentValue = _minValue;
        }
    } else if ((updatedState & DELTA_MASK) == INCREMENT_DELTA) {
        liveState.currentClick = DownClick;
        liveState.currentValue--;
        if (liveState.currentValue < _minValue) {
            liveState.currentValue = _maxValue;
        }
    }
    stateChanged = true;
}

}