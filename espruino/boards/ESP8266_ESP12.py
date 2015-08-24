#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Edited by Neil Kolban <kolban1@kolban.com>
# ----------------------------------------------------------------------------------------
# This file contains information for a specific board - the available pins, and where LEDs,
# Buttons, and other in-built peripherals are. It is used to build documentation as well
# as various source and header files for Espruino.
# ----------------------------------------------------------------------------------------

import pinutils;
info = {
 'name' : "ESP8266 ESP-12",
 'default_console' : "EV_SERIAL1",
 'variables' : 1023,
 'binary_name' : 'espruino_%v_esp8266_esp12',
};
chip = {
  'part' : "ESP8266",
  'family' : "ESP8266",
  'package' : "",
  'ram' : 80,
  'flash' : 512,
  'speed' : 80,
  'usart' : 1,
  'spi' : 0,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
};
# left-right, or top-bottom order
board = {
};
devices = {
};

board_css = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,7)  
  # just fake pins D0 .. D7
  return pins
