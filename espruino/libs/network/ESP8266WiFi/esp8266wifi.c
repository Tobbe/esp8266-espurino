#include <user_interface.h>
#include "esp8266wifi.h"

/**
 * Form a WiFi connection as a station to the access point using the
 * ssid supplied by ssid and the password supplied by password.
 */
void esp8266_wifi(char *ssid, char *password) {
	if (ssid == NULL || password == NULL) {
		return;
	}
	wifi_set_opmode_current(STATION_MODE);
	struct station_config stationConfig;
	strncpy(stationConfig.ssid, ssid, 32);
	if (password != NULL) {
		strncpy(stationConfig.password, password, 64);
	} else {
		strcpy(stationConfig.password, "");
	}
	wifi_station_set_config(&stationConfig);
	wifi_station_connect();
}
// End of file
