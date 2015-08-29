#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>

#define ESP8266_ON_ACCESS_POINTS "#accessPoints"

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jswrap_ESP8266WiFi.h"
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function
#include "network.h"

// Forward declaration of functions.
static void scanCB(void *arg, STATUS status);
static void wifiEventHandler(System_Event_t *event);
static void ipAddrToString(struct ip_addr addr, char *string);
static char *nullTerminateString(char *target, char *source, int sourceLength);

static JsVar *jsScanCallback;
static JsVar *jsWiFiEventCallback;

static void init() {
	// Intialize the ESP8266 board support.
}

// Let's define the JavaScript class that will contain our `world()` method. We'll call it `Hello`
/*JSON{
  "type"  : "class",
  "class" : "ESP8266WiFi"
}*/


/**
 * Connect the station to an access point.
 * o ssid - The network id of the access point.
 * o password - The password to use to connect to the access point.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "connect",
  "generate" : "jswrap_ESP8266WiFi_connect",
  "params"   : [
    ["ssid","JsVar","The network SSID"],
    ["password","JsVar","The password to the access point"]
  ]
}*/
void jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password) {

	if (jsv_ssid == NULL || !jsvIsString(jsv_ssid)) {
	    jsExceptionHere(JSET_ERROR, "No SSID.");
		return;
	}
	if (jsv_password == NULL || !jsvIsString(jsv_password)) {
	    jsExceptionHere(JSET_ERROR, "No password.");
		return;
	}

	// Create strings from the JsVars for the ESP8266 API calls.
	char ssid[33];
	int len = jsvGetString(jsv_ssid, ssid, sizeof(ssid)-1);
	ssid[len]='\0';
	char password[65];
	len = jsvGetString(jsv_password, password, sizeof(password)-1);
	password[len]='\0';

    jsiConsolePrintf("jswrap_ESP8266WiFi_connect: %s - %s\r\n", ssid, password);

    // Set the WiFi mode of the ESP8266
	wifi_set_opmode_current(STATION_MODE);

	jsiConsolePrintf("A\n");
	struct station_config stationConfig;
	memset(&stationConfig,0, sizeof(stationConfig));
	os_strncpy(stationConfig.ssid, ssid, 32);
	jsiConsolePrintf("B\n");
	if (password != NULL) {
		os_strncpy(stationConfig.password, password, 64);
	} else {
		os_strcpy(stationConfig.password, "");
	}
	jsiConsolePrintf("C\n");
	// Set the WiFi configuration
	wifi_station_set_config(&stationConfig);
	jsiConsolePrintf("D\n");

	wifi_station_connect();
	jsiConsolePrintf("E\n");
} // End of jswrap_ESP8266WiFi_connect


/**
 * Determine the list of access points available to us.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAccessPoints",
  "generate" : "jswrap_ESP8266WiFi_getAccessPoints",
  "params"   : [
    ["callback","JsVar","Function to call back when access points retrieved."]
  ]
}*/
void jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback) {
	jsiConsolePrint("> ESP8266WiFi_getAccessPoints\n");
	if (callback == NULL || !jsvIsFunction(callback)) {
	    jsExceptionHere(JSET_ERROR, "No callback.");
		return;
	}

	// Save the callback for the scan
	jsScanCallback = jsvLockAgainSafe(callback);

	// Ask the ESP8266 to perform a network scan after first entering
	// station mode.  This will result in an eventual callback which is where
	// Ensure we are in station mode
	wifi_set_opmode_current(STATION_MODE);

	// Request a scan of the network calling "scanCB" on completion
	wifi_station_scan(NULL, scanCB);

	jsiConsolePrint("< ESP8266WiFi_getAccessPoints\n");
} // End of jswrap_ESP8266WiFi_getAccessPoints

/**
 * Disconnect the station from the access point.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "disconnect",
  "generate" : "jswrap_ESP8266WiFi_disconnect"
}*/
void jswrap_ESP8266WiFi_disconnect() {
	wifi_station_disconnect();
} // End of jswrap_ESP8266WiFi_disconnect


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "restart",
  "generate" : "jswrap_ESP8266WiFi_restart"
}*/
void jswrap_ESP8266WiFi_restart() {
	system_restart();
} // End of jswrap_ESP8266WiFi_restart

/**
 * Register a callback function that will be invoked when a WiFi event is detected.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "onWiFiEvent",
  "generate" : "jswrap_ESP8266WiFi_onWiFiEvent",
  "params"   : [
    ["callback","JsVar","WiFi event callback"]
  ]
}*/
void jswrap_ESP8266WiFi_onWiFiEvent(JsVar *callback) {
	// If the callback is null
	if (callback == NULL || jsvIsNull(callback)) {
		if (jsWiFiEventCallback != NULL) {
			jsvUnLock(jsWiFiEventCallback);
		}
		jsWiFiEventCallback = NULL;
		return;
	}

	if (!jsvIsFunction(callback)) {
	    jsExceptionHere(JSET_ERROR, "No callback.");
		return;
	}

	// We are about to save a new global WiFi even callback handler.  If we have previously
	// had one, we need to unlock it so that we don't leak memory.
	if (jsWiFiEventCallback != NULL) {
		jsvUnLock(jsWiFiEventCallback);
	}

	// Save the global WiFi event callback handler.
	jsWiFiEventCallback = jsvLockAgainSafe(callback);

	// Register
	wifi_set_event_handler_cb(wifiEventHandler);
} // End of jswrap_ESP8266WiFi_onWiFiEvent

/**
 * Set whether or not the ESP8266 will perform an auto connect on startup.
 * A value of true means it will while a value of false means it won't.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "setAutoConnect",
  "generate" : "jswrap_ESP8266WiFi_setAutoConnect",
  "params"   : [
    ["autoconnect","JsVar","True if we wish to auto connect."]
  ]
}*/
void jswrap_ESP8266WiFi_setAutoConnect(JsVar *autoconnect) {
	os_printf("Auto connect is: %d\n", autoconnect);
	// Check that we have been passed a boolean ... if not, nothing to do here.
	if (!jsvIsBoolean(autoconnect)) {
		return;
	}

	uint8 newValue = jsvGetBool(autoconnect);
	os_printf("New value: %d\n", newValue);
	os_printf("jswrap_ESP8266WiFi_setAutoConnect -> Something breaks here :-(\n");

	uart_rx_intr_disable(0);
	wifi_station_set_auto_connect(newValue);
	uart_rx_intr_disable(1);
	os_printf("Autoconnect changed\n");
} // End of jswrap_ESP8266WiFi_setAutoconnect


/**
 * Retrieve whether or not the ESP8266 will perform an auto connect on startup.
 * A value of 1 means it will while a value of zero means it won't.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAutoConnect",
  "generate" : "jswrap_ESP8266WiFi_getAutoConnect",
  "return"   : ["JsVar","A boolean representing our auto connect status"]
}*/
JsVar *jswrap_ESP8266WiFi_getAutoConnect() {
	uint8 result = wifi_station_get_auto_connect();
	return jsvNewFromBool(result);
} // End of jswrap_ESP8266WiFi_getAutoconnect

/**
 * Retrieve the reset information that is stored when event the ESP8266 resets.
 * The result will be a JS object containing the details.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getRstInfo",
  "generate" : "jswrap_ESP8266WiFi_getRstInfo",
  "return"   : ["JsVar","A Restart Object"],
  "return_object" : "Restart"
}*/
JsVar *jswrap_ESP8266WiFi_getRstInfo() {
	struct rst_info* info = system_get_rst_info();
	JsVar *restartInfo = jspNewObject(NULL, "Restart");
	jsvObjectSetChild(restartInfo, "reason", jsvNewFromInteger(info->reason));
	jsvObjectSetChild(restartInfo, "exccause", jsvNewFromInteger(info->exccause));
	jsvObjectSetChild(restartInfo, "epc1", jsvNewFromInteger(info->epc1));
	jsvObjectSetChild(restartInfo, "epc2", jsvNewFromInteger(info->epc2));
	jsvObjectSetChild(restartInfo, "epc3", jsvNewFromInteger(info->epc3));
	jsvObjectSetChild(restartInfo, "excvaddr", jsvNewFromInteger(info->excvaddr));
	jsvObjectSetChild(restartInfo, "depc", jsvNewFromInteger(info->depc));
	return restartInfo;
} // End of jswrap_ESP8266WiFi_getRstInfo

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAddressAsString",
  "generate" : "jswrap_ESP8266WiFi_getAddressAsString",
  "params"   : [
    ["address","JsVar","An integer value representing an ipaddress."]
  ],
  "return"   : ["JsVar","A String"]
}*/
JsVar *jswrap_ESP8266WiFi_getAddressAsString(JsVar *address) {
	uint32 iAddress = (uint32)jsvGetInteger(address);
	os_printf("Out IP is %d\n", iAddress);
	return networkGetAddressAsString((uint8 *)&iAddress, 4, 10, '.');
} // End of jswrap_ESP8266WiFi_getAddressAsString

/**
 * Retrieve the IP information about this network interface and return a JS
 * object that contains its details.  The object will have the following
 * properties defined upon it:
 * o ip - The IP address of the interface.
 * o netmask - The netmask of the interface.
 * o gw - The gateway to reach when transmitting through the interface.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getIPInfo",
  "generate" : "jswrap_ESP8266WiFi_getIPInfo",
  "return"   : ["JsVar","A IPInfo Object"],
  "return_object" : "IPInfo"
}*/
JsVar *jswrap_ESP8266WiFi_getIPInfo() {
	struct ip_info info;
	wifi_get_ip_info(0, &info);

	JsVar *ipInfo = jspNewObject(NULL, "Restart");
	jsvObjectSetChild(ipInfo, "ip", jsvNewFromInteger(info.ip.addr));
	jsvObjectSetChild(ipInfo, "netmask", jsvNewFromInteger(info.netmask.addr));
	jsvObjectSetChild(ipInfo, "gw", jsvNewFromInteger(info.gw.addr));
	return ipInfo;
} // End of jswrap_ESP8266WiFi_getIPInfo


/**
 * Query the station configuration and return a JS object that represents the
 * current settings.  The object will have the following properties:
 *
 * o ssid - The network identity of the access point
 * o password - The password to use to connect to the access point
 *
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getStationConfig",
  "generate" : "jswrap_ESP8266WiFi_getStationConfig",
  "return"   : ["JsVar","A Station Config"],
  "return_object" : "StationConfig"
}*/
JsVar *jswrap_ESP8266WiFi_getStationConfig() {
	struct station_config config;
	wifi_station_get_config(&config);
	JsVar *jsConfig = jspNewObject(NULL, "StationConfig");
	//char ssid[33];
	//nullTerminateString(ssid, (char *)config.ssid, 32);
	jsvObjectSetChild(jsConfig, "ssid", jsvNewFromString((char *)config.ssid));
	//char password[65];
	//nullTerminateString(password, (char *)config.password, 64);
	jsvObjectSetChild(jsConfig, "password", jsvNewFromString((char *)config.password));
	return jsConfig;
} // End of jswrap_ESP8266WiFi_getStationConfig

static char *nullTerminateString(char *target, char *source, int sourceLength) {
	os_strncpy(target, sourceLength, source);
	target[sourceLength-1] = '\0';
	return target;
}

static void setupJsNetwork() {
	JsNetwork net;
	networkCreate(&net, JSNETWORKTYPE_ESP8266_BOARD);
	networkSet(&net);
}



/**
 * Callback function that is invoked at the culmination of a scan.
 */
static void scanCB(void *arg, STATUS status) {
	/**
	 * Create a JsVar that is an array of JS objects where each JS object represents a
	 * retrieved access point set of information.   The structure of a record will be:
	 * o authMode
	 * o isHidden
	 * o rssi
	 * o channel
	 * o ssid
	 * When the array has been built, invoke the callback function passing in the array
	 * of records.
	 */

	// Create the Empty JS array that will be passed as a parameter to the callback.
	JsVar *accessPointArray = jsvNewArray(NULL, 0);
	struct bss_info *bssInfo;

	bssInfo = (struct bss_info *)arg;
	// skip the first in the chain … it is invalid
	bssInfo = STAILQ_NEXT(bssInfo, next);
	while(bssInfo != NULL) {
		// Add a new object to the JS array that will be passed as a parameter to
		// the callback.  The ESP8266 bssInfo structure contains the following:
		// ---
		// uint8 bssid[6]
		// uint8 ssid[32]
		// uint8 channel
		// sint8 rssi – The received signal strength indication
		// AUTH_MODE authmode
		//	Open = 0
		//	WEP = 1
		//	WPA_PSK = 2
		//	WPA2_PSK = 3
		//	WPA_WPA2_PSK = 4
		// uint8 is_hidden
		// sint16 freq_offset
		// ---
		// Create, populate and add a child ...
		JsVar *currentAccessPoint = jspNewObject(NULL, "AccessPoint");
		jsvObjectSetChild(currentAccessPoint, "rssi", jsvNewFromInteger(bssInfo->rssi) );
		jsvObjectSetChild(currentAccessPoint, "channel", jsvNewFromInteger(bssInfo->channel));
		jsvObjectSetChild(currentAccessPoint, "authMode", jsvNewFromInteger(bssInfo->authmode));
		jsvObjectSetChild(currentAccessPoint, "isHidden", jsvNewFromBool(bssInfo->is_hidden));
		// The SSID may **NOT** be NULL terminated ... so handle that.
		char ssid[sizeof(bssInfo->ssid) + 1];
		os_strncpy(ssid, bssInfo->ssid, sizeof(bssInfo->ssid));
		ssid[sizeof(ssid)-1] = '\0';
		jsvObjectSetChild(currentAccessPoint, "ssid", jsvNewFromString(ssid));

		// Add the new record to the array
		jsvArrayPush(accessPointArray, currentAccessPoint);

		os_printf("ssid: %s\n", bssInfo->ssid);
		bssInfo = STAILQ_NEXT(bssInfo, next);
	} // End of loop over the records.

	// We have now completed the scan callback, so now we can invoke the JS callback.
	JsVar *params[1];
	params[0] = accessPointArray;
	jsiQueueEvents(NULL, jsScanCallback, params, 1);
	jsvUnLock(jsScanCallback);
} // End of scanCB


/**
 * Invoke the JavaScript callback to notify the program that an ESP8266
 * WiFi event has occurred.
 */
static void sendWifiEvent(uint32 eventType, JsVar *details) {
	// We need to check that we actually have an event callback handler because
	// it might have been disabled/removed.
	if (jsWiFiEventCallback == NULL) {
		return;
	}

	// Build a callback event.
	JsVar *params[2];
	params[0] = jsvNewFromInteger(eventType);
	params[1] = details;
	jsiQueueEvents(NULL, jsWiFiEventCallback, params, 2);
} // End of sendWifiEvent


/**
 * ESP8266 WiFi Event handler.  This function is called by the ESP8266
 * environment when significant events happend related to the WiFi environment.
 * The event handler is registered with a call to wifi_set_event_handler_cb()
 * that is provided by the ESP8266 SDK.
 */
static void wifiEventHandler(System_Event_t *event) {
	switch(event->event) {
	case EVENT_STAMODE_CONNECTED:
		os_printf("Event: EVENT_STAMODE_CONNECTED");
		sendWifiEvent(event->event, jsvNewNull());
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("Event: EVENT_STAMODE_DISCONNECTED");
		JsVar *details = jspNewObject(NULL, "EventDetails");
		jsvObjectSetChild(details, "reason", jsvNewFromInteger(event->event_info.disconnected.reason));
		char ssid[33];
		memcpy(ssid, event->event_info.disconnected.ssid, event->event_info.disconnected.ssid_len);
		ssid[ event->event_info.disconnected.ssid_len] = '\0';
		sendWifiEvent(event->event, details);
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("Event: EVENT_STAMODE_AUTHMODE_CHANGE");
		sendWifiEvent(event->event, jsvNewNull());
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("Event: EVENT_STAMODE_GOT_IP");
		sendWifiEvent(event->event, jsvNewNull());
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STACONNECTED");
		sendWifiEvent(event->event, jsvNewNull());
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STADISCONNECTED");
		sendWifiEvent(event->event, jsvNewNull());
		break;
	default:
		os_printf("Unexpected event: %d\r\n", event->event);
		sendWifiEvent(event->event, jsvNewNull());
		break;
	}
} // End of wifiEventHandler

// Note: This may be a duplicate ... it appears that we may have an existing function
// in network.c which does exactly this and more!!
//
static void ipAddrToString(struct ip_addr addr, char *string) {
	os_sprintf(string, "%d.%d.%d.%d", ((char *)&addr)[0], ((char *)&addr)[1], ((char *)&addr)[2], ((char *)&addr)[3]);
} // End of ipAddrToString
