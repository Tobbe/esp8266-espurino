/*
 * This file is part of Espruino/ESP8266, a JavaScript interpreter for ESP8266
 *
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDE_ESP8266_WIFI_H_
#define INCLUDE_ESP8266_WIFI_H_

void ESP8266_setupWiFi(char *ssid, char *password, void (* gotIpCallback)());

#endif /* INCLUDE_ESP8266_WIFI_H_ */
