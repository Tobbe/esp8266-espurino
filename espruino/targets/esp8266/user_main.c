/*
 * This file is part of Espruino/ESP8266, a JavaScript interpreter for ESP8266
 *
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <user_interface.h>
#include <osapi.h>
#include <driver/uart.h>
#include <telnet.h>

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long  int64_t;

#include <jsdevices.h>

// --- Constants
#define TASK_QUEUE_LENGTH 10
#define APP_PRIO USER_TASK_PRIO_1

// --- Forward definitions
static void mainLoop();

// --- File local variables
static uint32 lastTime;
static os_event_t taskQueue[TASK_QUEUE_LENGTH];

// --- Functions

/**
 * The event handler for ESP8266 tasks as created by system_os_post().
 */
static void eventHandler(os_event_t *pEvent) {
	mainLoop();
} // End of eventHandler

/**
 * A callback function to be invoked when a line has been entered on the telnet client.
 * Here we want to pass that line to the JS parser for processing.
 */
static void telnetLineCB(char *line) {
	jsiConsolePrintf("LineCB: %s", line);
	// Pass the line to the interactive module ...

	jshPushIOCharEvents(jsiGetConsoleDevice(), line, strlen(line));
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

static void mainLoop() {
	jsiLoop();
	if (system_get_time() - lastTime > 1000 * 1000 * 5) {
		lastTime = system_get_time();
		os_printf("tick: %d\n", jshGetSystemTime());
	}

	// Setup for another callback
	system_os_post(APP_PRIO, (os_signal_t)1, 0);
} // End of mainLoop

/**
 * The ESP8266 provides a mechanism to register a callback that is invoked when initialization
 * of the ESP8266 is complete.  This is the implementation of that callback.  At this point
 * we can assume that the ESP8266 is fully ready to do work for us.
 */
static void initDone() {
	os_printf("initDone invoked\n");
	jshInit(); // Initialize the hardware
	jsvInit(); // Initialize the variables
	jsiInit(); // Initialize the interactive subsystem
	jsiConsolePrintf("\nAbout to setup WiFi\n");
	ESP8266_setupWiFi("sweetie", "kolban12", gotIpCallback);

	system_os_task(eventHandler, APP_PRIO, taskQueue, TASK_QUEUE_LENGTH);
	uint32 lastTime = system_get_time();
	system_os_post(APP_PRIO, (os_signal_t)1, 0);
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
}
// End of file
