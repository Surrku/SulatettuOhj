#include "uart.h"
#include "dispatch.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <string.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define U_STACKSIZE 500
#define U_PRIORITY 5


static const struct device *const uart_dev =
    DEVICE_DT_GET(UART_DEVICE_NODE);

void uart_task(void *unused1, void *unused2, void *unused3);

K_THREAD_DEFINE(uart_thread,U_STACKSIZE,uart_task,NULL,NULL,NULL,U_PRIORITY,0,0);

int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			// printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				printk("UART msg: %s\n", uart_msg);
                
				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}

                                //copies UART message to dispatcher data
                                snprintf(buf->msg, 20, "%s", uart_msg);

                                //puts dispatcher data to FIFO buffer
                                k_fifo_put(&dispatcher_fifo, buf);

				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);

				// Clear UART message buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
		}
		k_msleep(10);
	}
	return 0;
}