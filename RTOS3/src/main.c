//koodia siivottu ja jaettu eri heaaderien alle. 
//viikkotehtävä 4seen liittyvät koodit löytyvät:
//---> main.c
//---> leds.c
//---> dispatch.c
//screenshotit timer tulostuksista repon Screenshot kansiosta
//tehtävä 4 -> 1p -> terminaaliin tulostuu yksittäiseen valotaskiin kulunut aika & kokonaisaika

//tehtävä 5 -> 2p -> testikeisseihin lisätty boundary value-testit, kuvankaappaus Screenshot kansiossa
//timeparser.c ja .h lisätty RTOS3seen, aikakeskeytys toimii terminaalin kautta ja laittaa punaisen ledin päälle
//viikkotehtävään liittyvät muokkaukset tehty:
//---> main.c
//---> dispatch.c
//lisättyjä testikeissejä: merkkijonon pituus, merkkijonossa vain numeroita, merkkijono ei ole NULL



#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/timing/timing.h>

#include "leds.h"
#include "btn.h"
#include "uart.h"
#include "dispatch.h"
#include "TimeParser.h"

// Timer initializations
struct k_timer timer;
void timer_handler(struct k_timer *timer_id);

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/
int main(void)
{

        timing_init();
        timing_start();

        init_led();

		k_timer_init(&timer, timer_handler, NULL);
    
        int ret = init_button();
	        if (ret < 0) {
	        return 0;
	}

	ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}

        return 0;
}

void timer_handler(struct k_timer *timer_id) {
	printk("Timer run out\n");
	run_red_led();
}




