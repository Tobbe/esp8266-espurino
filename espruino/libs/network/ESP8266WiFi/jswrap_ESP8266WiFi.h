/*
 * jswrap_ESP8266WiFi.h
 *
 *  Created on: Aug 26, 2015
 *      Author: kolban
 */

#ifndef LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_
#define LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_
#include "jsvar.h"

void   jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password);
void   jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback);
void   jswrap_ESP8266WiFi_disconnect();
void   jswrap_ESP8266WiFi_restart();
JsVar *jswrap_ESP8266WiFi_getRstInfo();
JsVar *jswrap_ESP8266WiFi_getIPInfo();
void   jswrap_ESP8266WiFi_setAutoConnect(JsVar *autoconnect);
JsVar *jswrap_ESP8266WiFi_getAutoConnect();
JsVar *jswrap_ESP8266WiFi_getStationConfig();
void   jswrap_ESP8266WiFi_onWiFiEvent(JsVar *callback);
JsVar *jswrap_ESP8266WiFi_getAddressAsString(JsVar *address);
void   jswrap_ESP8266WiFi_init();
JsVar *jswrap_ESP8266WiFi_getConnectStatus();
JsVar *jswrap_ESP8266WiFi_socketConnect(JsVar *options, JsVar *callback);
void   jswrap_ESP8266WiFi_socketEnd(JsVar *socket, JsVar *data);
void   jswrap_ESP8266WiFi_ping(JsVar *ipAddr);

#endif /* LIBS_NETWORK_ESP8266WIFI_JSWRAP_ESP8266WIFI_H_ */
