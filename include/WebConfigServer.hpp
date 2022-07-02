#pragma once
//
// WebConfig 
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
#include "WebServer.h"

namespace WebConfigServer{

void initialize();

void processClient();

void handle_setChannel();

void handle_led1On();

void handle_led1Off();

void handle_led2On();

void handle_led2Off();

void handle_OnConnect();

void handle_NotFound();

String SendHTML(String channel, uint8_t led1Status, uint8_t led2Status);

}