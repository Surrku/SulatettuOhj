#include "dispatch.h"
#include "leds.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/timing/timing.h>
#include <inttypes.h>

#define D_STACKSIZE 500
#define D_PRIORITY 5

K_THREAD_DEFINE(dis_thread, D_STACKSIZE, dispatcher_task, NULL, NULL, NULL, D_PRIORITY, 0,0);
K_FIFO_DEFINE(dispatcher_fifo);

int seq_paused = 0;
K_MUTEX_DEFINE(seq_mutex);
K_CONDVAR_DEFINE(seq_cond);

void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);

        timing_t sequence_start = timing_counter_get();

        int cnt=0;
                while (sequence[cnt] != 0) {

                        k_mutex_lock(&seq_mutex, K_FOREVER);

                        while (seq_paused == 1) {
                                k_condvar_wait(&seq_cond,
                                                &seq_mutex,
                                                K_FOREVER);
                        }

                        k_mutex_unlock(&seq_mutex);

                        if (sequence[cnt] == 'R') {
                            printk("RED\n");
                            run_red_led();
                        }
                        else if (sequence[cnt] == 'Y') {
                            printk("YELLOW\n");
                            run_yellow_led();
                        }
                        else if (sequence[cnt] == 'G') {
                            printk("GREEN\n");
                            run_green_led();
                        }
                        cnt++;

                }
        timing_t sequence_end = timing_counter_get();

        uint64_t sequence_ns =
            timing_cycles_to_ns(
                timing_cycles_get(&sequence_start, &sequence_end));

        uint64_t sequence_us = sequence_ns / 1000;

        printk("Total task time: %" PRIu64 "\n", sequence_us);
	}
}

