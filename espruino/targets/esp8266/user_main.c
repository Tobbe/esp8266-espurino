#include <osapi.h>
#include <driver/uart.h>

void user_rf_pre_init() {

}

void user_init() {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("\nControl has arrived at user_main!\n");
}
// End of file
