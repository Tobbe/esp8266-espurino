/*
 * This file is part of Espruino/ESP8266, a JavaScript interpreter for ESP8266
 *
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
//#include "espmissingincludes.h"
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

// The callback function to be invoked when we have been issued an IP address
static void (* m_gotIpCallback)();

/**
 * Callback handler for the different events that can be generated from the WiFi subsystem.
 */
static void wifiEventHandler(System_Event_t *event) {
	switch (event->event) {
	case EVENT_STAMODE_CONNECTED:
		os_printf("Event: EVENT_STAMODE_CONNECTED");
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("Event: EVENT_STAMODE_DISCONNECTED");
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("Event: EVENT_STAMODE_AUTHMODE_CHANGE");
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("Event: EVENT_STAMODE_GOT_IP");
		if (m_gotIpCallback != NULL) {
			m_gotIpCallback();
		}
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STACONNECTED");
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STADISCONNECTED");
		break;
	default:
		os_printf("Unexpected event: %d\r\n", event->event);
		break;
	}
} // End of wifiEventHandler


/**
 * Initialize the ESP8266 WiFi subsystem by making us a WiFi station with a given userid
 * and password.
 */
void ESP8266_setupWiFi(char *ssid, char *password, void (* gotIpCallback)()) {
	struct station_config stationConfig;
	m_gotIpCallback = gotIpCallback;
	wifi_set_opmode(STATION_MODE);
	wifi_set_event_handler_cb(wifiEventHandler);
	strncpy((char *)stationConfig.ssid, ssid, 32);
	strncpy((char *)stationConfig.password, password, 64);
	wifi_station_set_config_current(&stationConfig);
	os_printf("station connect called\n");
	wifi_station_connect();
} // End of ESP8266_setupWiFi
// End of file
