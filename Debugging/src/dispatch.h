#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <zephyr/kernel.h>

struct data_t {
    void *fifo_reserved;
    char msg[20];
};

extern struct k_mutex seq_mutex;
extern struct k_condvar seq_cond;
extern int seq_paused;
extern struct k_fifo dispatcher_fifo;

void dispatcher_task(void *unused1, void *unused2, void *unused3);

#endif
