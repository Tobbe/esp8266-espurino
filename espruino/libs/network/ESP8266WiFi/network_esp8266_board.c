/*
 * network_esp8266_board.c

 *
 *  Created on: Aug 29, 2015
 *      Author: kolban
 */

/**
 * This file contains the implementation of the ESP8266_BOARD network interfaces at the TCP/IP
 * level.
 *
 */
// ESP8266 specific includes
#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <espconn.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "network_esp8266_board.h"
#define MAX_SOCKETS (10)

extern void suspendMainLoop(uint32 interval);

/**
 * void initSocketData()
 * int getNextFreeSocket()
 * void releaseSocket(int s)
 * struct socketData *getSocketData(int s)
 */

// -------------------------------------------------
struct socketData {
	bool inUse;
	struct espconn espconn;
};

static struct socketData socketArray[MAX_SOCKETS];
static esp_tcp tcpArray[MAX_SOCKETS];

/**
 * Get the next free socket.
 */
static int getNextFreeSocket() {
	for (int i=0; i<MAX_SOCKETS; i++) {
		if (socketArray[i].inUse == 0) {
			socketArray[i].inUse = 1;
			return i;
		}
	}
	return(-1);
} // End of getNextFreeSocket

static void releaseSocket(int s) {
	socketArray[s].inUse = 0;
}

static struct socketData *getSocketData(int s) {
	assert(s >=0 && s<MAX_SOCKETS);
	return &socketArray[s];
}

static void initSocketData() {
	for (int i=0; i<MAX_SOCKETS; i++) {
		struct socketData *socketData = getSocketData(i);
		socketData->inUse = 0;
		socketData->espconn.type = ESPCONN_TCP;
		socketData->espconn.state = ESPCONN_NONE;
		socketData->espconn.proto.tcp = &tcpArray[i];
	}
} // End of initSocketData()

void netInit_esp8266_board() {
	initSocketData();
}

static void connectCB(void *arg) {
	os_printf("connectCB\n");
}

static void disconnectCB(void *arg) {
	os_printf("disconnectCB\n");
}

/**
 * Although this is called reconnect by Espressif, this is really an error handler
 * routine.  It will be called when an error is detected.
 */
static void reconnectCB(void *arg, sint8 err) {
	os_printf("> reconnectCB:  Error code is: %d\n", err);
}

static void sentCB(void *arg) {
	os_printf("sentCB\n");
}

static void recvCB(void *arg, char *pData, unsigned short len) {
	os_printf("recvCB\n");
}

// -------------------------------------------------

void netSetCallbacks_esp8266_board(JsNetwork *net) {
	//jsiConsolePrint("> netSetCallbacks_esp8266_board\n");
	  net->idle          = net_ESP8266_BOARD_idle;
	  net->checkError    = net_ESP8266_BOARD_checkError;
	  net->createsocket  = net_ESP8266_BOARD_createSocket;
	  net->closesocket   = net_ESP8266_BOARD_closeSocket;
	  net->accept        = net_ESP8266_BOARD_accept;
	  net->gethostbyname = net_ESP8266_BOARD_gethostbyname;
	  net->recv          = net_ESP8266_BOARD_recv;
	  net->send          = net_ESP8266_BOARD_send;
	//jsiConsolePrint("< netSetCallbacks_esp8266_board\n");
} // End of netSetCallbacks_esp8266_board

/**
 * net        - The Network we are going to use to create the socket.
 * serverSckt - The socket that we are now going to start accepting requests upon.
 */
int net_ESP8266_BOARD_accept(JsNetwork *net, int serverSckt) {
	os_printf("> net_ESP8266_BOARD_accept\n");
	return 0;
} // End of net_ESP8266_BOARD_accept

/**
 * Receive data from the network device.
 * net  - The Network we are going to use to create the socket.
 * recv - The socket from which we are to receive data.
 * buf  - The storage buffer into which we will receive data.
 * len  - The length of the buffer.
 *
 * Returns the number of bytes received which may be 0 and -1 if there was an error.
 */
int net_ESP8266_BOARD_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
	//os_printf("> net_ESP8266_BOARD_recv\n");
	return 0;
} // End of net_ESP8266_BOARD_recv.


/**
 * net  - The Network we are going to use to create the socket.
 * sckt - The socket over which we will send data.
 * buf  - The buffer containing the data to be sent.
 * len  - The length of data in the buffer to send.
 */
int net_ESP8266_BOARD_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
	os_printf("> net_ESP8266_BOARD_send\n");
	// Make a call to epsconn_send.
	return 0;
} // End of net_ESP8266_BOARD_send

/**
 * net - The Network we are going to use to create the socket.
 */
void net_ESP8266_BOARD_idle(JsNetwork *net) {
	// Don't echo here because it is called continuously
	//os_printf("> net_ESP8266_BOARD_idle\n");
} // End of net_ESP8266_BOARD_idle

/**
 * net - The Network we are going to use to create the socket.
 *
 * Returns true if there are NO errors.
 */
bool net_ESP8266_BOARD_checkError(JsNetwork *net) {
	//os_printf("> net_ESP8266_BOARD_checkError\n");
	return 1;
} // End of net_ESP8266_BOARD_checkError

/**
 * net - The Network we are going to use to create the socket.
 * ipAddress - The address of the partner of the socket.
 * port - The port number that the partner is listening upon.
 *
 * if ipAddress == 0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success.
 */
int net_ESP8266_BOARD_createSocket(JsNetwork *net, uint32_t ipAddress, unsigned short port) {
	os_printf("> net_ESP8266_BOARD_createSocket: host: %d.%d.%d.%d, port:%d \n", ((char *)(&ipAddress))[0], ((char *)(&ipAddress))[1], ((char *)(&ipAddress))[2], ((char *)(&ipAddress))[3], port);
	int sckt = getNextFreeSocket();
	if (sckt < 0) { // No free socket
		return -1;
	}

	struct socketData *socketData = getSocketData(sckt);
	struct espconn *pEspconn = &(socketData->espconn);
	pEspconn->type = ESPCONN_TCP;
	pEspconn->state = ESPCONN_NONE;
	pEspconn->proto.tcp = &tcpArray[sckt];
	pEspconn->proto.tcp->remote_port = port;
	pEspconn->proto.tcp->local_port = espconn_port();
	//pEspconn->proto.tcp->local_port = 0;
	*(uint32_t *)(pEspconn->proto.tcp->remote_ip) = ipAddress;
	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	os_memcpy(pEspconn->proto.tcp->local_ip, &ipconfig.ip, 4);
	espconn_regist_connectcb(pEspconn, connectCB);
	espconn_regist_disconcb(pEspconn, disconnectCB);
	espconn_regist_reconcb(pEspconn, reconnectCB);
	espconn_regist_sentcb(pEspconn, sentCB);
	espconn_regist_recvcb(pEspconn, recvCB);
	int rc = espconn_connect(pEspconn);
	if (rc != 0) {
		os_printf("Err: net_ESP8266_BOARD_createSocket -> espconn_connect returned: %d.  Using local port: %d\n", rc, pEspconn->proto.tcp->local_port);
	}

	// Make a call to espconn_connect.
	os_printf("< net_ESP8266_BOARD_createSocket\n");
	return sckt;
} // End of net_ESP8266_BOARD_createSocket

/**
 * net - The Network we are going to use to create the socket.
 * sckt - The socket to be closed.
 */
void net_ESP8266_BOARD_closeSocket(JsNetwork *net, int sckt) {
	os_printf("> net_ESP8266_BOARD_closeSocket\n");
	assert(getSocketData(sckt)->inUse == 1);
	releaseSocket(sckt);
	// Make a call to espconn_delete
	// Make a call to espconn_disconnect
} // End of net_ESP8266_BOARD_closeSocket

/**
 * Get an IP address from a name. Sets `outIp` to 0 on failure.
 *
 * net      - The Network we are going to use to create the socket.
 * hostName - The string representing the hostname we wish to lookup.
 * outIp    - The address into which the resolved IP address will be stored.
 */
void net_ESP8266_BOARD_gethostbyname(JsNetwork *net, char *hostName, uint32_t *outIp) {
	os_printf("> net_ESP8266_BOARD_gethostbyname\n");
	*outIp = 0x00000000;
} // End of net_ESP8266_BOARD_gethostbyname
// End of file
