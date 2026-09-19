//koodia siivottu ja jaettu eri heaaderien alle. 
//viikkotehtävä 4seen liittyvät koodit löytyvät:
//---> main.c
//---> leds.c
//---> dispatch.c
//screenshotit timer tulostuksista repon Screenshot kansiosta
//tehtävä 4 -> 1p -> terminaaliin tulostuu yksittäiseen valotaskiin kulunut aika & kokonaisaika


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



