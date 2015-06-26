

#ifndef TIVA_C_WS2812B_WSLIB_H_
#define TIVA_C_WS2812B_WSLIB_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;

void ws_init(uint16_t num_leds);
void ws_update();
void ws_set_pixel(uint32_t pixel, uint8_t r, uint8_t g, uint8_t b);
void ws_set_pixel(uint32_t pixel, rgb_t col);
bool ws_busy();


#endif /* TIVA_C_WS2812B_WSLIB_H_ */
