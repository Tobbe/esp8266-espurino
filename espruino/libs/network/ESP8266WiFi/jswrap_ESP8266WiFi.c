#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>

#define ESP8266_ON_ACCESS_POINTS "#accessPoints"

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jswrap_ESP8266WiFi.h"
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

static void scanCB(void *arg, STATUS status);

static JsVar *jsScanCallback;

static void init() {
	// Intialize the ESP8266 board support.
}

// Let's define the JavaScript class that will contain our `world()` method. We'll call it `Hello`
/*JSON{
  "type" : "class",
  "class" : "ESP8266WiFi"
}*/

/*JSON{
  "type" : "staticmethod",
  "class" : "ESP8266WiFi",
  "name" : "connect",
  "generate" : "jswrap_ESP8266WiFi_connect",
  "params" : [
    ["ssid","JsVar","The network SSID"],
    ["password","JsVar","The password to the access point"]
  ]
}*/
void jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password) {

	if (jsv_ssid == NULL || !jsvIsString(jsv_ssid)) {
	    jsExceptionHere(JSET_ERROR, "No SSID");
		return;
	}
	if (jsv_password == NULL || !jsvIsString(jsv_password)) {
	    jsExceptionHere(JSET_ERROR, "No password");
		return;
	}

	char ssid[33];
	int len = jsvGetString(jsv_ssid, ssid, sizeof(ssid)-1);
	ssid[len]='\0';
	char password[65];
	len = jsvGetString(jsv_password, password, sizeof(password)-1);
	password[len]='\0';

    jsiConsolePrintf("jswrap_ESP8266WiFi_connect: %s - %s\r\n", ssid, password);
	if (ssid == NULL || password == NULL) {
		return;
	}
	wifi_set_opmode_current(STATION_MODE);
	struct station_config stationConfig;
	os_strncpy(stationConfig.ssid, ssid, 32);
	if (password != NULL) {
		os_strncpy(stationConfig.password, password, 64);
	} else {
		os_strcpy(stationConfig.password, "");
	}
	wifi_station_set_config(&stationConfig);
	wifi_station_connect();
} // End of jswrap_ESP8266WiFi_connect

/*JSON{
  "type" : "staticmethod",
  "class" : "ESP8266WiFi",
  "name" : "getAccessPoints",
  "generate" : "jswrap_ESP8266WiFi_getAccessPoints",
    "params" : [
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
	// XXX
	JsVar *accessPointArray = jsvNewArray(NULL, 0);
	JsVar *currentAccessPoint;

	int count = 0;
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
		currentAccessPoint = jspNewObject(NULL, "AccessPoint");
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
