/*
 * jswrap_ESP8266WiFi.h
 *
 *  Created on: Aug 26, 2015
 *      Author: kolban
 */

#ifndef LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_
#define LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_
#include "jsvar.h"

void jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password, JsVar *callback);
void jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback);
void jswrap_ESP8266WiFi_disconnect();
void jswrap_ESP8266WiFi_restart();
JsVar *jswrap_ESP8266WiFi_getRstInfo();
JsVar *jswrap_ESP8266WiFi_getIPInfo();
#endif /* LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_ */
