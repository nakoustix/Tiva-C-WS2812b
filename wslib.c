
#include "wslib.h"
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
#include <string.h>
#include <stdlib.h>

#define PW_HIGH	72
#define PW_LOW	28

static uint8_t *frame_buffer;

static uint32_t buf_size;

static uint32_t draw_count;
static uint8_t draw_mask;
static bool last_pixel = false;
static bool busy = false;
static uint8_t reset_count = 0;
static uint16_t strip_len;
static uint8_t r_offset = 0;
static uint8_t g_offset = 1;
static uint8_t b_offset = 2;

void pwm_int()
{
	uint32_t period;
	PWMGenIntClear(PWM1_BASE, PWM_GEN_3, PWM_INT_CNT_ZERO);

	if( frame_buffer[draw_count] & draw_mask )
		period = PW_HIGH;
	else
		period = PW_LOW;

	if( last_pixel )
	{
		// Do 42 periods with duty = 0 to hold the line low for
		// 52.5 us (minimum reset time is 50 us; 1s / 800kHz * 42 cycles = 52.5 us)
		// Maybe it works with only 41..?
		if( ++reset_count >= 42 )
		{
			PWMGenDisable(PWM1_BASE, PWM_GEN_3);
			busy = false;
		}
	}
	else
	{
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, period);
		draw_mask <<= 1;
		if( ! draw_mask )
		{
			draw_mask = 1;
			if(++draw_count >= buf_size)
			{
				last_pixel = true;
				reset_count = 0;
				PWMPulseWidthSet( PWM1_BASE, PWM_OUT_7, 0 );
			}
		}
	}

}

void ws_init(uint16_t num_leds)
{
	strip_len = num_leds;
	buf_size = strip_len * 3;

	frame_buffer = (uint8_t *) malloc( buf_size );

	memset(frame_buffer, 0, buf_size);


	// Set clock source for PWM peripheral
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
	// Enable peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Configure pins for peripheral operation
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	GPIOPinTypePWM(GPIO_PORTF_BASE, (1 << 3));
	// Configure the PWM generator
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	// Set the new period
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 100);
	// Set dutycycle to 50%
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 72);

	// Register the interrupt handler...
	PWMGenIntRegister(PWM1_BASE, PWM_GEN_3, pwm_int);
	// ...and enable the interrupt
	PWMIntEnable(PWM1_BASE, PWM_INT_GEN_3);
	IntEnable(INT_PWM1_3);
	PWMGenIntTrigEnable(PWM1_BASE, PWM_GEN_3, PWM_INT_CNT_ZERO);

	//PWMGenEnable(PWM1_BASE, PWM_GEN_3);
	//PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
}

bool ws_busy()
{
	return busy;
}

void ws_update()
{
	uint32_t period;
	busy = true;
	draw_mask = 1;
	draw_count = 0;
	last_pixel = false;
	if( frame_buffer[draw_count] & draw_mask )
	{
		period = PW_HIGH;
	}
	else
	{
		period = PW_LOW;
	}
	draw_mask <<= 1;
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, period);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
}

void ws_set_pixel(uint32_t pixel, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t offset = pixel * 3;
	frame_buffer[offset] = r;
	frame_buffer[offset+1] = g;
	frame_buffer[offset+2] = b;
}

void ws_set_pixel_rgb(uint32_t pixel, rgb_t col)
{
	uint32_t offset = pixel * 3;
	frame_buffer[offset + r_offset] = col.r;
	frame_buffer[offset + g_offset] = col.g;
	frame_buffer[offset + b_offset] = col.b;
}

void ws_set_color_order( color_order_e order )
{
	switch( order )
	{
	case CR_RGB:
	{
		r_offset = 0;
		g_offset = 1;
		b_offset = 2;
		break;
	}
	case CR_RBG:
	{
		r_offset = 0;
		g_offset = 2;
		b_offset = 1;
		break;
	}
	case CR_GRB:
	{
		r_offset = 1;
		g_offset = 0;
		b_offset = 2;
		break;
	}
	case CR_GBR:
	{
		r_offset = 2;
		g_offset = 0;
		b_offset = 1;
		break;
	}
	}
}
