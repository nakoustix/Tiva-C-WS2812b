

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123fe6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "wslib.h"

#define STRIP_LEN	20

int
main(void)
{
	int i;
	// Set MCLK frequency to 80MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlDelay(10000);


    ws_init( STRIP_LEN );
    for(i = 0; i < STRIP_LEN; i++)
    {
    	ws_set_pixel(i, 255 - i * 10, i * 10, 0);
    }


    while(1)
    {
    	ws_update();
    	while( ws_busy() );
    }
}
