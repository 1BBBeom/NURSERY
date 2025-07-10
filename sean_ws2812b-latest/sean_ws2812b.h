#ifndef WS2812B_H
#define WS2812B_H

#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "msh.h"

#ifndef WS2812B_SPI_BUS_NAME
#define WS2812B_SPI_BUS_NAME "spi4"
#endif

#ifndef WS2812B_SPI_DEV_NAME
#define WS2812B_SPI_DEV_NAME "spi40"
#endif

#ifndef WS2812B_SPI_CS_PORT
#define WS2812B_SPI_CS_PORT GPIOE
#endif

#ifndef WS2812B_SPI_CS_PIN
#define WS2812B_SPI_CS_PIN GPIO_PIN_6
#endif

#ifndef WS2812B_LED_NUMS
#define WS2812B_LED_NUMS 30
#endif

#define WS2812B_RGB_BITS 24
#define WS2812B_CODE_0 0xC0
#define WS2812B_CODE_1 0xF0

#define WS2812B_COLOR_BLACK 0x000000
#define WS2812B_COLOR_RED 0xFF0000
#define WS2812B_COLOR_ORANGE 0xF08784
#define WS2812B_COLOR_YELLOW 0xFF7F27
#define WS2812B_COLOR_GREEN 0x7FFF00
#define WS2812B_COLOR_CYAN 0x00FFFF
#define WS2812B_COLOR_BLUE 0x0000FF
#define WS2812B_COLOR_PURPLE 0x8B00FF
#define WS2812B_COLOR_WHITE 0xFFFFFF

int ws2812b_init(void);
void ws2812b_set_brightness(uint8_t brightness);
void ws2812b_set_color(uint16_t index, uint32_t color);
void ws2812b_show(void);
void ws2812b_clear(void);
void ws2812b_set_all(uint32_t color);

#endif
