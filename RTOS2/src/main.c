//viikko 2 - 2p - valosekvenssi toimii terminaalin kautta syöttämällä R(red), Y(yellow) ja/tai G(green). 
//leditaskit eivät pyöri superloopilla, vaan Thread APIa käyttäen

//RTOS 1&2 yhdistetty. nappi 1 -> pausettaa sarjaportin kautta tulevan sekvenssin
// nappi 2 - 4 -> yksittäiset ledit päälle ja pois
// nappi 5 -> nothing

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <zephyr/drivers/uart.h>

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)

//int led_state = 0; // 0 = idle, 1 = red, 2 = yellow, 3 = green, 4 = pause, 5 = only yellow
//int direction = 0; // 0 = down, 1 = up
int old_state = 0; //0 = running, 1 = paused
int seq_paused = 0;

K_MUTEX_DEFINE(seq_mutex);
K_CONDVAR_DEFINE(seq_cond);

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {0});

void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
//void dispatcher_task(void *, void *, void*);
//void uart_task(void *, void *, void*);
//K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
//K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
//K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);

struct k_thread red_thread_data;
struct k_thread green_thread_data;
struct k_thread yellow_thread_data;

static struct gpio_callback button_0_data;
static struct gpio_callback button_1_data;
static struct gpio_callback button_2_data;
static struct gpio_callback button_3_data;
static struct gpio_callback button_4_data;

K_THREAD_STACK_DEFINE(red_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(green_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(yellow_stack, STACKSIZE);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
};

/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

//Pause button
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
        printk("Button 0 pressed\n");

        k_mutex_lock(&seq_mutex, K_NO_WAIT);

        if (seq_paused == 0) {
                seq_paused = 1;
                printk("Sequence paused\n");
        } else {
                seq_paused = 0;
                printk("Sequence resumed\n");
                k_condvar_broadcast(&seq_cond);
        }

        k_mutex_unlock(&seq_mutex);

}
//Red led on/off button
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 1 pressed\n");

	gpio_pin_toggle_dt(&red);
		
	int pin_state = gpio_pin_get_dt(&red);
	if (pin_state > 0) {
		printk("Red led on\n");
	} else {
		printk("Red led off\n");
	}
	
}

//Yellow led on/off button
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 2 pressed\n");


	gpio_pin_toggle_dt(&green);
	gpio_pin_toggle_dt(&red);

        int red_state = gpio_pin_get_dt(&red);
        int green_state = gpio_pin_get_dt(&green);
		
	if (red_state > 0 && green_state > 0) {
		printk("Yellow led on\n");
	} else {
		printk("Yellow led off\n");
	}
	
}

//Green led on/off button
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 3 pressed\n");


	gpio_pin_toggle_dt(&green);
		
	int pin_state = gpio_pin_get_dt(&green);
	if (pin_state > 0) {
		printk("Green led on\n");
	} else {
		printk("Green led off\n");
	}
	
}

//Blinking yellow light button
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 4 pressed\n");
        /*if (led_state == 5) {
	        led_state = old_state;
	        printk("Yellow-only stopped\n");
	} else {
		old_state = led_state;
		led_state = 5;
		printk("Yellow-only started\n");
	}*/

}

/********************
 * Main task
 */
int main(void)
{

    init_led();
    
    int ret = init_button();
	if (ret < 0) {
	        return 0;
	}

	init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}

	return 0;
}

/********************
 * UART task
 */
static void uart_task(void *unused1, void *unused2, void *unused3)
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

/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);

        int cnt=0;
                while (sequence[cnt] != 0) {

                        k_mutex_lock(&seq_mutex, K_NO_WAIT);

                        while (seq_paused == 1) {
                                k_condvar_wait(&seq_cond,
                                                &seq_mutex,
                                                K_FOREVER);
                        }

                        k_mutex_unlock(&seq_mutex);

                        if (sequence[cnt] == 'R') {
                            printk("RED\n");
	                        k_thread_create(
				        &red_thread_data,
					red_stack,
					K_THREAD_STACK_SIZEOF(red_stack),
					red_led_task,
					NULL, NULL, NULL,
					PRIORITY, 
					0,
					K_NO_WAIT
				);
				k_thread_join(&red_thread_data, K_FOREVER);

                        }
                        else if (sequence[cnt] == 'Y') {
                            printk("YELLOW\n");
	                        k_thread_create(
					&yellow_thread_data,
					yellow_stack,
					K_THREAD_STACK_SIZEOF(yellow_stack),
					yellow_led_task,
					NULL, NULL, NULL,
					PRIORITY, 
					0,
					K_NO_WAIT
				);
				k_thread_join(&yellow_thread_data, K_FOREVER);

                        }
                        else if (sequence[cnt] == 'G') {
                            printk("GREEN\n");
                            k_thread_create(
				        &green_thread_data,
				        green_stack,
					K_THREAD_STACK_SIZEOF(green_stack),
					green_led_task,
					NULL, NULL, NULL,
					PRIORITY, 
					0,
					K_NO_WAIT
				);
				k_thread_join(&green_thread_data, K_FOREVER); 

                        }
                        cnt++;

                }
        // You need to:
        // Parse color and time from the fifo data
        // Example
        //    char color = sequence[0];
        //    int time = atoi(sequence+2);
		//    printk("Data: %c %d\n", color, time);
        // Send the parsed color information to tasks using fifo
        // Use release signal to control sequence or k_yield
	}
}

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

	printk("Red led ready\n");
		// 1. set led on 
	gpio_pin_set_dt(&red,1);
	printk("Red on\n");
	k_sleep(K_SECONDS(1));
	// 3. set led off
	gpio_pin_set_dt(&red,0);
	printk("Red off\n");
	k_sleep(K_SECONDS(1));

}

// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	printk("Yellow led ready\n");
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


}


// Task to handle green led
void green_led_task(void *, void *, void*) {

	printk("Green led ready\n");
	// 1. set led on 
	gpio_pin_set_dt(&green,1);
	printk("Green on\n");
	k_sleep(K_SECONDS(1));
	// 3. set led off
	gpio_pin_set_dt(&green,0);
	printk("Green off\n");
	k_sleep(K_SECONDS(1));

}


int init_button() {

	//BUTTON 0
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");

	//BUTTON 1
	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up button 1 ok\n");

	//BUTTON 2
	if (!gpio_is_ready_dt(&button_2)) {
		printk("Error: button 2 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
	printk("Set up button 2 ok\n");

	//BUTTON 3
	if (!gpio_is_ready_dt(&button_3)) {
		printk("Error: button 3 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
	printk("Set up button 3 ok\n");

	//BUTTON 4
	if (!gpio_is_ready_dt(&button_4)) {
		printk("Error: button 4 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up button 4 ok\n");
	
	return 0;
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);