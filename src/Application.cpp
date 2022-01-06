//
// Application 
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

#include "Application.hpp"
#include "Buttons.hpp"

namespace grt {
namespace Application {

using namespace lr;

void processButtonPresses(){
    switch (Buttons::getNextButtonPress()) {
    case Buttons::Up:
        Serial.println("Up");
        break;
    case Buttons::longUp:
        Serial.println("Long Press Up");
        break;
    case Buttons::OK:
        Serial.println("Ok");
        break;
    case Buttons::longOK:
        Serial.println("Long Press Ok");
        break;
    case Buttons::Down:
        Serial.println("Down");
        break;
    case Buttons::longDown:
        Serial.println("Long Press Up");
        break;
    default:
        // nothing to be done
        break;
    }
}

}   
}