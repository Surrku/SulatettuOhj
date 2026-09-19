#include "leds.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>
#include <inttypes.h>

#define LED_STACKSIZE 500
#define LED_PRIORITY 5

static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static struct k_thread red_thread_data;
static struct k_thread green_thread_data;
static struct k_thread yellow_thread_data;

K_THREAD_STACK_DEFINE(red_stack, LED_STACKSIZE);
K_THREAD_STACK_DEFINE(green_stack, LED_STACKSIZE);
K_THREAD_STACK_DEFINE(yellow_stack, LED_STACKSIZE);

bool red_state = false;
bool yellow_state = false;
bool green_state = false;

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&red,0);

        ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&green,0);

	printk("Led initialized ok\n");
	
	return 0;
        
}

// Task to handle red led
void red_led_task(void *, void *, void*) {

	//printk("Red led ready\n");
    timing_t red_start_time = timing_counter_get();
	// 1. set led on 
	gpio_pin_set_dt(&red,1);
	printk("Red on\n");
	k_sleep(K_SECONDS(1));
	// 3. set led off
	gpio_pin_set_dt(&red,0);
	printk("Red off\n");
	k_sleep(K_SECONDS(1));

    timing_t red_end_time = timing_counter_get();
    uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&red_start_time, & red_end_time));
    uint64_t timing_us = timing_ns / 1000;
    printk("Red task: %" PRIu64 "\n", timing_us);

}

// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	//printk("Yellow led ready\n");
    timing_t yellow_start_time = timing_counter_get();
	// 1. set led on 
	gpio_pin_set_dt(&red,1);
	gpio_pin_set_dt(&green,1);
	printk("Yellow on\n");
	k_sleep(K_SECONDS(1));
		// 3. set led off
	gpio_pin_set_dt(&red,0);
	gpio_pin_set_dt(&green,0);
	printk("Yellow off\n");
	k_sleep(K_SECONDS(1));

    timing_t yellow_end_time = timing_counter_get();
    uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&yellow_start_time, & yellow_end_time));
    uint64_t timing_us = timing_ns / 1000;
    printk("Yellow task: %" PRIu64 "\n", timing_us);


}


// Task to handle green led
void green_led_task(void *, void *, void*) {

	//printk("Green led ready\n");
    timing_t green_start_time = timing_counter_get();
	// 1. set led on 
	gpio_pin_set_dt(&green,1);
	printk("Green on\n");
	k_sleep(K_SECONDS(1));
	// 3. set led off
	gpio_pin_set_dt(&green,0);
	printk("Green off\n");
	k_sleep(K_SECONDS(1));

    timing_t green_end_time = timing_counter_get();
    uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&green_start_time, & green_end_time));
    uint64_t timing_us = timing_ns / 1000;
    printk("Green task: %" PRIu64 "\n", timing_us);

}

int run_red_led(void)
{
    k_thread_create(
        &red_thread_data,
        red_stack,
        K_THREAD_STACK_SIZEOF(red_stack),
        red_led_task,
        NULL, NULL, NULL,
        LED_PRIORITY,
        0,
        K_NO_WAIT
    );

    return k_thread_join(&red_thread_data, K_FOREVER);
}

int run_yellow_led(void)
{
    k_thread_create(
        &yellow_thread_data,
        yellow_stack,
        K_THREAD_STACK_SIZEOF(yellow_stack),
        yellow_led_task,
        NULL, NULL, NULL,
        LED_PRIORITY,
        0,
        K_NO_WAIT
    );

    return k_thread_join(&yellow_thread_data, K_FOREVER);
}

int run_green_led(void)
{
    k_thread_create(
        &green_thread_data,
        green_stack,
        K_THREAD_STACK_SIZEOF(green_stack),
        green_led_task,
        NULL, NULL, NULL,
        LED_PRIORITY,
        0,
        K_NO_WAIT
    );

    return k_thread_join(&green_thread_data, K_FOREVER);
}