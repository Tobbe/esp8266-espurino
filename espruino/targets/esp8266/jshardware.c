#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <gpio.h>
#include <mem.h>
#include <driver/uart.h>

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

/**
 * Transmit all the characters in the transmit buffer.
 *
 * Note: In the first pass of this implementation we write all the data to UART1
 * however for proper and full implementation we should also handle UART0.
 */
void esp8266_uartTransmitAll(IOEventFlags device) {
	int c = jshGetCharToTransmit(device);
	while (c >= 0) {
		//os_printf("%c", c);
		uart_tx_one_char(0, c);
		c = jshGetCharToTransmit(device);
	} // No more characters to transmit
} // End of esp8266_transmitAll

// ----------------------------------------------------------------------------

IOEventFlags pinToEVEXTI(Pin pin) {
	return (IOEventFlags) 0;
}

/**
 * Initialize the ESP8266 hardware environment.
 */
void jshInit() {
	// A call to jshInitDevices is architected as something we have to do.
	jshInitDevices();
} // End of jshInit

void jshReset() {
	// TODO
}

void jshKill() {
	// TODO
}

/**
 * Hardware idle processing.
 */
void jshIdle() {
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
	const char *code = "ESP8266";
	strncpy((char *) data, code, maxChars);
	return strlen(code);
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
	// TODO
}

void jshInterruptOn() {
	// TODO
}

void jshDelayMicroseconds(int microsec) {
	if (0 < microsec) {
		os_delay_us(microsec);
	}
}

static uint8_t PERIPHS[] = {
PERIPHS_IO_MUX_GPIO0_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_U0TXD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO2_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_U0RXD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO4_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO5_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_CLK_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA0_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA1_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA2_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA3_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_CMD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTDI_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTCK_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTMS_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTDO_U - PERIPHS_IO_MUX };
#define FUNC_SPI 1
#define FUNC_GPIO 3
#define FUNC_UART 4
static uint8_t function(JshPinState state) {
	switch (state) {
	case JSHPINSTATE_GPIO_OUT:
	case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
	case JSHPINSTATE_GPIO_IN:
	case JSHPINSTATE_GPIO_IN_PULLUP:
	case JSHPINSTATE_GPIO_IN_PULLDOWN:
		return FUNC_GPIO;
	case JSHPINSTATE_USART_OUT:
	case JSHPINSTATE_USART_IN:
		return FUNC_UART;
	case JSHPINSTATE_I2C:
		return FUNC_SPI;
	case JSHPINSTATE_AF_OUT:
	case JSHPINSTATE_AF_OUT_OPENDRAIN:
	case JSHPINSTATE_DAC_OUT:
	case JSHPINSTATE_ADC_IN:
	default:
		return 0;
	}
}
void jshPinSetState(Pin pin, JshPinState state) {
	jsiConsolePrintf("jshPinSetState %d, %d\n", pin, state);

	assert(pin < 16);
	int periph = PERIPHS_IO_MUX + PERIPHS[pin];
	PIN_PULLUP_DIS(periph);
	//PIN_PULLDWN_DIS(periph);

	uint8_t primary_func =
			pin < 6 ?
					(PERIPHS_IO_MUX_U0TXD_U == pin
							|| PERIPHS_IO_MUX_U0RXD_U == pin) ?
							FUNC_UART : FUNC_GPIO
					: 0;
	uint8_t select_func = function(state);
	PIN_FUNC_SELECT(periph, primary_func == select_func ? 0 : select_func);

	switch (state) {
	case JSHPINSTATE_GPIO_OUT:
	case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
		//case JSHPINSTATE_AF_OUT:
		//case JSHPINSTATE_AF_OUT_OPENDRAIN:
		//case JSHPINSTATE_USART_OUT:
		//case JSHPINSTATE_DAC_OUT:
		gpio_output_set(0, 0x1 << pin, 0x1 << pin, 0);
		break;
	case JSHPINSTATE_ADC_IN:
	case JSHPINSTATE_USART_IN:
	case JSHPINSTATE_I2C:
		PIN_PULLUP_EN(periph);
		break;
	case JSHPINSTATE_GPIO_IN_PULLUP:
		PIN_PULLUP_EN(periph);
		//case JSHPINSTATE_GPIO_IN_PULLDOWN: if (JSHPINSTATE_GPIO_IN_PULLDOWN == pin) PIN_PULLDWN_EN(periph);
	case JSHPINSTATE_GPIO_IN:
		gpio_output_set(0, 0, 0, 0x1 << pin);
		break;
	default:
		;
	}
}

JshPinState jshPinGetState(Pin pin) {
	os_printf("jshPinGetState %d\n", pin);
	return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
//	jsiConsolePrintf("jshPinSetValue %d, %d\n", pin, value);
	GPIO_OUTPUT_SET(pin, value);
}

bool jshPinGetValue(Pin pin) {
//	jsiConsolePrintf("jshPinGetValue %d, %d\n", pin, GPIO_INPUT_GET(pin));
	return GPIO_INPUT_GET(pin);
}

bool jshIsDeviceInitialised(IOEventFlags device) {
	os_printf("jshIsDeviceInitialised %d\n", device);
	return true;
}

bool jshIsUSBSERIALConnected() {
	os_printf("jshIsUSBSERIALConnected\n");
	return true;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
//	jsiConsolePrintf("jshGetTimeFromMilliseconds %d, %f\n", (JsSysTime)(ms * 1000.0), ms);
	return (JsSysTime) (ms * 1000.0 + 0.5);
}

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
//	jsiConsolePrintf("jshGetMillisecondsFromTime %d, %f\n", time, (JsVarFloat)time / 1000.0);
	return (JsVarFloat) time / 1000.0;
} // End of jshGetMillisecondsFromTime

/**
 * Return the current time in microseconds.
 */
JsSysTime jshGetSystemTime() { // in us
	return system_get_time();
} // End of jshGetSystemTime


/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime time) {
	os_printf("SetSystemTime: %d\n", time);
} // End of jshSetSystemTime

// ----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
//	jsiConsolePrintf("jshPinAnalog: %d\n", pin);
	return (JsVarFloat) system_adc_read();
}

int jshPinAnalogFast(Pin pin) {
//	jsiConsolePrintf("jshPinAnalogFast: %d\n", pin);
	return NAN;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq) { // if freq<=0, the default is used
//	jsiConsolePrintf("jshPinAnalogOutput: %d, %d, %d\n", pin, (int)value, (int)freq);
//pwm_set(pin, value < 0.0f ? 0 : 255.0f < value ? 255 : (uint8_t)value);
	return 0;
}

void jshSetOutputValue(JshPinFunction func, int value) {
	os_printf("jshSetOutputValue %d %d\n", func, value);
}

void jshEnableWatchDog(JsVarFloat timeout) {
	os_printf("jshEnableWatchDog %0.3f\n", timeout);
}

bool jshGetWatchedPinState(IOEventFlags device) {
	//jsiConsolePrintf("jshGetWatchedPinState %d", device);
	return false;
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
	if (jshIsPinValid(pin)) {
		jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
		jshPinSetValue(pin, value);
		jshDelayMicroseconds(jshGetTimeFromMilliseconds(time));
		jshPinSetValue(pin, !value);
	} else
		jsError("Invalid pin!");
}

bool jshCanWatch(Pin pin) {
	return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
	if (jshIsPinValid(pin)) {
	} else
		jsError("Invalid pin!");
	return EV_NONE;
}

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
	os_printf("jshGetCurrentPinFunction %d\n", pin);
	return JSH_NOTHING;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
	return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
} // jshUSARTKick

/**
 * Kick a device into action (if required). For instance we may need
 * to set up interrupts.  In this ESP8266 implementation, we transmit all the
 * data that can be found associated with the device.
 */
void jshUSARTKick(IOEventFlags device) {
	esp8266_uartTransmitAll(device);
} // End of jshUSARTKick

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
	return NAN;
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
	jshSPISend(device, data >> 8);
	jshSPISend(device, data & 255);
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

/** Wait until SPI send is finished, */
void jshSPIWait(IOEventFlags device) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes,
		const unsigned char *data, bool sendStop) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes,
		unsigned char *data, bool sendStop) {
}

void jshSaveToFlash() {
	os_printf("jshSaveToFlash\n");
}

void jshLoadFromFlash() {
	os_printf("jshLoadFromFlash\n");
}

bool jshFlashContainsCode() {
	os_printf("jshFlashContainsCode\n");
	return false;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
	int time = (int) timeUntilWake;
//	os_printf("jshSleep %d\n", time);
	jshDelayMicroseconds(time);
	return true;
}

void jshUtilTimerDisable() {
	os_printf("jshUtilTimerDisable\n");
}

void jshUtilTimerReschedule(JsSysTime period) {
	os_printf("jshUtilTimerReschedule %d\n", period);
}

void jshUtilTimerStart(JsSysTime period) {
	os_printf("jshUtilTimerStart %d\n", period);
}

JsVarFloat jshReadTemperature() {
	return NAN;
}
;
JsVarFloat jshReadVRef() {
	return NAN;
}
;

unsigned int jshGetRandomNumber() {
	return rand();
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
}

void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
}

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
	return false;
}

void jshFlashErasePage(uint32_t addr) {
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

/**
 * Callback for end of runtime.  This should never be called and has been
 * added to satisfy the linker.
 */
void _exit(int status) {
} // End of _exit
// End of file
