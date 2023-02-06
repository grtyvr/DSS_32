//
// Encoders
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
#include "ArduinoHeaders.hpp"
#include "Encoders.hpp"
#include "NewEncoder.h"
#include "AS5048.hpp"
#include "Loop.hpp"
#include "Preferences.h"

namespace grt {
namespace Encoders{

class CustomEncoder: public NewEncoder {
  public:
    CustomEncoder() :
      NewEncoder() {
    }
    CustomEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE) :
      NewEncoder(aPin, bPin, minValue, maxValue, initalValue, type) {
    }
    virtual ~CustomEncoder() {
    }

  protected:
    virtual void updateValue(uint8_t updatedState);
};

void ESP_ISR CustomEncoder::updateValue(uint8_t updatedState) {
  if ((updatedState & DELTA_MASK) == INCREMENT_DELTA) {
    liveState.currentClick = UpClick;
    liveState.currentValue++;
    if (liveState.currentValue > _maxValue) {
      liveState.currentValue = _minValue;
    }
  } else if ((updatedState & DELTA_MASK) == DECREMENT_DELTA) {
    liveState.currentClick = DownClick;
    liveState.currentValue--;
    if (liveState.currentValue < _minValue) {
      liveState.currentValue = _maxValue;
    }
  }
  stateChanged = true;
}


int al_A_pin = 34;
int al_B_pin = 19;
int al_min_val = 0;
int al_max_tics = 9544;
int al_start_val = 0;
int az_A_pin = 5;
int az_B_pin = 18;
int az_min_val = 0;
int az_max_tics = 5216;
int az_start_val = 0;

CustomEncoder alEnc(al_A_pin, al_B_pin, al_min_val, al_max_tics, al_start_val, FULL_PULSE);
CustomEncoder azEnc(az_A_pin, az_B_pin, az_min_val, az_max_tics, az_start_val, FULL_PULSE);

int16_t prevAlValue;
int16_t prevAzValue;
//
///////////////////////////////////////////////////////////////////////////////

/// Initialise this module
///
void initialize(int al_max_tics, int az_max_tics){
  CustomEncoder::EncoderState AlState;
  CustomEncoder::EncoderState AzState;

  alEnc.configure(al_A_pin, al_B_pin, al_min_val, al_max_tics, al_start_val, FULL_PULSE);
  azEnc.configure(az_A_pin, az_B_pin, az_min_val, az_max_tics, az_start_val, FULL_PULSE);

  delay(2000);
  Serial.println("Starting");
  if (!alEnc.begin()) {
    Serial.println("Altitude Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
    while (1) {
      yield();
    }
  } else {
    alEnc.getState(AlState);
    Serial.print("Altitude Encoder Successfully Started at value = ");
    prevAlValue = AlState.currentValue;
    Serial.println(prevAlValue);
  }

  if (!azEnc.begin()) {
    Serial.println("Azimuth Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
    while (1) {
      yield();
    }
  } else {
    azEnc.getState(AzState);
    Serial.print("Azimuth Encoder Successfully Started at value = ");
    prevAzValue = AzState.currentValue;
    Serial.println(prevAlValue);
  }



}

/// Get the encoder readings.
///
Position getPosition(){
//    int16_t curAlVal;
//    int16_t curAzVal;

    CustomEncoder::EncoderState currentAlEncoderState;
    CustomEncoder::EncoderState currentAzEncoderState;

    alEnc.getState(currentAlEncoderState);
    azEnc.getState(currentAzEncoderState);

    Position curPos = {
        currentAlEncoderState.currentValue,
        currentAzEncoderState.currentValue
    };
    return curPos;
}

Position getMaxTics(){
    Position maxTics = {
        al_max_tics,
        az_max_tics
    };
    return maxTics;
}

Position setMaxTics(Position pos){
  al_max_tics = pos.altitude;
  az_max_tics = pos.azimuth;
  Preferences preferences;
  preferences.begin("prefs", false);
  preferences.putInt("alMaxTics", al_max_tics);
  preferences.putInt("azMaxTics", az_max_tics);
  preferences.end();
  Serial.println(al_A_pin);
  if (alEnc.enabled()){
    alEnc.end();
    alEnc.configure(al_A_pin, al_B_pin, al_min_val, al_max_tics, al_start_val, FULL_PULSE);
    if (!alEnc.begin()) {
      Serial.println("Altitude Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
      while (1) {
        yield();
      }
    }
  }
  if (azEnc.enabled()){
    azEnc.end();
    azEnc.configure(az_A_pin, az_B_pin, az_min_val, az_max_tics, az_start_val, FULL_PULSE);
    if (!azEnc.begin()) {
      Serial.println("Azimuth Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
      while (1) {
        yield();
      }
    }
  }
  return getMaxTics();
}
}
}