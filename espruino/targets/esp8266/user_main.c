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
typedef long long int64_t;

#include <jsdevices.h>
#include "ESP8266_board.h"

// --- Constants
#define WIFI_SSID "kolbanwifi"
#define WIFI_PASSWORD "zdpu867$"

#define CONNECT_WIFI 0

#define TASK_QUEUE_LENGTH 10

// Should we introduce a ticker to say we are still alive?
//#define EPS8266_BOARD_HEARTBEAT

// --- Forward definitions
static void mainLoop();

// --- File local variables
static uint32 lastTime;
// The task queue for the app
static os_event_t taskAppQueue[TASK_QUEUE_LENGTH];

static bool suspendMainLoopFlag = false;
static os_timer_t mainLoopSuspendTimer;

// --- Functions

void suspendMainLoop(uint32 interval) {
	suspendMainLoopFlag = true;
	os_timer_arm(&mainLoopSuspendTimer, interval, 0 /* No repeat */);
}

static void queueMainLoop() {
	system_os_post(TASK_APP_QUEUE, TASK_APP_MAINLOOP, 0);
}

static void enableMainLoop() {
	suspendMainLoopFlag = false;
	queueMainLoop();
}

/**
 * The event handler for ESP8266 tasks as created by system_os_post() on the TASK_APP_QUEUE.
 */
static void eventHandler(os_event_t *pEvent) {
	char pBuffer[100];
	switch (pEvent->sig) {
	// Handle the main loop event.
	case TASK_APP_MAINLOOP:
		mainLoop();
		break;
	// Handle the event to process received data.
	case TASK_APP_RX_DATA: {
		// Get the data from the UART RX buffer.  If the size of the returned data is
		// not zero, then push it onto the Espruino processing queue for characters.
		int size = getRXBuffer(pBuffer, sizeof(pBuffer));
		if (size > 0) {
			jshPushIOCharEvents(jsiGetConsoleDevice(), pBuffer, size);
		}
	}
		break;
	// Handle the unknown event type.
	default:
		os_printf("user_main: eventHandler: Unknown task type: %d",
				pEvent->sig);
		break;
	}
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
	if (suspendMainLoopFlag == true) {
		return;
	}
	jsiLoop();

#ifdef EPS8266_BOARD_HEARTBEAT
	if (system_get_time() - lastTime > 1000 * 1000 * 5) {
		lastTime = system_get_time();
		os_printf("tick: %d\n", jshGetSystemTime());
	}
#endif

	// Setup for another callback
	queueMainLoop();
} // End of mainLoop

/**
 * The ESP8266 provides a mechanism to register a callback that is invoked when initialization
 * of the ESP8266 is complete.  This is the implementation of that callback.  At this point
 * we can assume that the ESP8266 is fully ready to do work for us.
 */
static void initDone() {
	os_printf("initDone invoked\n");

	// Discard any junk data in the input as this is a boot.
	//uart_rx_discard();

	uint32 lastTime = system_get_time();

	jshInit(); // Initialize the hardware
	jsvInit(); // Initialize the variables
	jsiInit(); // Initialize the interactive subsystem

	// Register the event handlers.
	system_os_task(eventHandler, TASK_APP_QUEUE, taskAppQueue,
	TASK_QUEUE_LENGTH);

	// Post the first event to get us going.
	queueMainLoop();

	return;
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
	UART_SetPrintPort(1);
	// Print an initialization message (just for sanity)
	os_printf("\nControl has arrived at user_main!\n");
	// Register the ESP8266 initialization callback.
	system_init_done_cb(initDone);
	// Do NOT attempt to auto connect to an access point.
	//wifi_station_set_auto_connect(0);
	os_timer_setfn(&mainLoopSuspendTimer, enableMainLoop);
}
// End of file
