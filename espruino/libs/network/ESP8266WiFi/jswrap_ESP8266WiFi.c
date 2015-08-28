#include <c_types.h>
#include <user_interface.h>
#include "esp8266wifi.h"

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jswrap_ESP8266WiFi.h"
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

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
	char ssid[10];
	int len = jsvGetString(jsv_ssid, ssid, sizeof(ssid)-1);
	ssid[len]='\0';
	char password[65];
	len = jsvGetString(jsv_password, password, sizeof(password)-1);
	password[len]='\0';

    jsiConsolePrintf("jswrap_ESP8266WiFi_connect: %s - %s\r\n", ssid, password);
    esp8266_wifi(ssid, password);
}
