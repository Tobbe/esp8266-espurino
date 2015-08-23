/*
 * This file is part of Espruino/ESP8266, a JavaScript interpreter for ESP8266
 *
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osapi.h>
#include <driver/uart.h>
#include <telnet.h>

/**
 * A callback function to be invoked when a line has been entered on the telnet client.
 * Here we want to pass that line to the JS parser for processing.
 */
static void telnetLineCB(char *line) {
	jsiConsolePrintf("LineCB: %s", line);
	//jspEvaluate(line, true);
	//jsiDumpState();
	telnet_send("JS> ");
} // End of lineCB

/**
 * When we have been allocated a TCP/IP address, this function is called back.  Obviously
 * there is little we can do at the network level until we have an IP.
 */
static void gotIpCallback() {
	telnet_startListening(telnetLineCB);
} // End of gotIpCallback

/**
 * The ESP8266 provides a mechanism to register a callback that is invoked when initialization
 * of the ESP8266 is complete.  This is the implementation of that callback.  At this point
 * we can assume that the ESP8266 is fully ready to do work for us.
 */
static void initDone() {
	os_printf("initDone");
	jsiConsolePrintf("\nAbout to setup WiFi\n");
	ESP8266_setupWiFi("sweetie", "kolban12", gotIpCallback);
} // End of initDone

/**
 * This is a required function needed for ESP8266 SDK.  In 99.999% of the instances, this function
 * needs to be present but have no body.  It isn't 100% known what this function does other than
 * provide an architected callback during initializations.  However, its purpose is unknown.
 */
void user_rf_pre_init() {
}

/**
 * This is the main entry point in an ESP8266 application.  It is where the logic of
 * ESP8266 starts.
 */
void user_init() {
	// Initialize the UART devices
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	// Print an initialization message (just for sanity)
	os_printf("\nControl has arrived at user_main!\n");
	// Register the ESP8266 initialization callback.
	system_init_done_cb(initDone);
	// Do NOT attempt to auto connect to an access point.
	wifi_station_set_auto_connect(0);


	char a[10];
	strncat(a, "hello", 5);
	int i = strlen(a);
	strcpy(a, "xxx");
}
// End of file
