#include "sean_ws2812b.h"
#include "rtdbg.h"
#include <math.h>
#include <string.h>

static struct rt_spi_device *ws2812b_spi_dev = RT_NULL;
static uint8_t ws2812b_buffer[WS2812B_LED_NUMS * WS2812B_RGB_BITS] = {0};
static uint32_t ws2812b_colors[WS2812B_LED_NUMS] = {0};
static uint8_t gamma_table[256];
static uint8_t global_brightness = 255;

static void ws2812b_generate_gamma_table(void)
{
    for (int i = 0; i < 256; i++)
    {
        gamma_table[i] = (uint8_t)(powf((float)i / 255.0f, 2.8f) * 255.0f + 0.5f);
    }
}

static void ws2812b_apply_brightness(uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = gamma_table[(*r * global_brightness) >> 8];
    *g = gamma_table[(*g * global_brightness) >> 8];
    *b = gamma_table[(*b * global_brightness) >> 8];
}

static void ws2812b_encode_color(uint8_t r, uint8_t g, uint8_t b, uint8_t *buffer)
{
    uint8_t bits[WS2812B_RGB_BITS] = {0};

    for (int i = 0; i < 8; i++)
        bits[7 - i] = (g & (1 << i)) ? WS2812B_CODE_1 : WS2812B_CODE_0;
    for (int i = 0; i < 8; i++)
        bits[15 - i] = (r & (1 << i)) ? WS2812B_CODE_1 : WS2812B_CODE_0;
    for (int i = 0; i < 8; i++)
        bits[23 - i] = (b & (1 << i)) ? WS2812B_CODE_1 : WS2812B_CODE_0;

    memcpy(buffer, bits, WS2812B_RGB_BITS);
}

int ws2812b_init(void)
{
    if (rt_hw_spi_device_attach(WS2812B_SPI_BUS_NAME, WS2812B_SPI_DEV_NAME, WS2812B_SPI_CS_PORT, WS2812B_SPI_CS_PIN) != RT_EOK)
    {
        LOG_E("Failed to attach SPI device %s.", WS2812B_SPI_DEV_NAME);
        return -RT_ERROR;
    }

    ws2812b_spi_dev = (struct rt_spi_device *)rt_device_find(WS2812B_SPI_DEV_NAME);
    if (!ws2812b_spi_dev)
    {
        LOG_E("Cannot find SPI device %s.", WS2812B_SPI_DEV_NAME);
        return -RT_ERROR;
    }

    struct rt_spi_configuration cfg = {0};
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
    cfg.max_hz = 6 * 1000 * 1000;
    cfg.data_width = 8;

    rt_spi_configure(ws2812b_spi_dev, &cfg);

    memset(ws2812b_buffer, WS2812B_CODE_0, sizeof(ws2812b_buffer));
    memset(ws2812b_colors, 0, sizeof(ws2812b_colors));

    ws2812b_generate_gamma_table();

    return RT_EOK;
}

void ws2812b_set_brightness(uint8_t brightness)
{
    if (brightness > 255)
        brightness = 255;
    global_brightness = brightness;

    for (uint16_t i = 0; i < WS2812B_LED_NUMS; i++)
    {
        ws2812b_set_color(i, ws2812b_colors[i]);
    }
}

void ws2812b_set_color(uint16_t index, uint32_t color)
{
    if (index >= WS2812B_LED_NUMS)
    {
        LOG_E("LED index %d out of range.", index);
        return;
    }

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    ws2812b_apply_brightness(&r, &g, &b);
    ws2812b_encode_color(r, g, b, &ws2812b_buffer[index * WS2812B_RGB_BITS]);
    ws2812b_colors[index] = color;
}

void ws2812b_show(void)
{
    rt_spi_send(ws2812b_spi_dev, ws2812b_buffer, sizeof(ws2812b_buffer));
}

void ws2812b_clear(void)
{
    memset(ws2812b_buffer, WS2812B_CODE_0, sizeof(ws2812b_buffer));
    memset(ws2812b_colors, 0, sizeof(ws2812b_colors));
    ws2812b_show();
}

void ws2812b_set_all(uint32_t color)
{
    for (uint16_t i = 0; i < WS2812B_LED_NUMS; i++)
    {
        ws2812b_set_color(i, color);
    }
}