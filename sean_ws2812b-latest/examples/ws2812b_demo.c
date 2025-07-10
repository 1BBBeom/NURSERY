#include <rtthread.h>
#include <sean_ws2812b.h>
#include <msh.h>

#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY 20
#define THREAD_TIMESLICE 5

/* 颜色表，使用宏定义 */
static uint32_t color_table[] = {
    WS2812B_COLOR_RED,    // 红色
    WS2812B_COLOR_ORANGE, // 橙色
    WS2812B_COLOR_YELLOW, // 黄色
    WS2812B_COLOR_GREEN,  // 绿色
    WS2812B_COLOR_CYAN,   // 青色
    WS2812B_COLOR_BLUE,   // 蓝色
    WS2812B_COLOR_PURPLE, // 紫色
    WS2812B_COLOR_WHITE   // 白色
};

/* 全部灯切换颜色 */
static void ws2812b_full_color(void)
{
    for (uint8_t i = 0; i < sizeof(color_table) / sizeof(color_table[0]); i++)
    {
        ws2812b_set_all(color_table[i]);
        ws2812b_show();
        rt_thread_mdelay(1000);
    }
}

/* 流水灯效果 */
static void ws2812b_running(void)
{
    ws2812b_clear();
    for (uint16_t i = 0; i < WS2812B_LED_NUMS; i++)
    {
        ws2812b_clear();
        ws2812b_set_color(i, WS2812B_COLOR_BLUE);
        ws2812b_show();
        rt_thread_mdelay(50);
    }
}

/* 亮度渐变测试 */
static void ws2812b_brightness(void)
{
    ws2812b_set_all(WS2812B_COLOR_GREEN);
    ws2812b_show();

    for (uint8_t brightness = 255; brightness > 10; brightness -= 5)
    {
        ws2812b_set_brightness(brightness);
        ws2812b_show();
        rt_thread_mdelay(50);
    }

    for (uint8_t brightness = 10; brightness < 255; brightness += 5)
    {
        ws2812b_set_brightness(brightness);
        ws2812b_show();
        rt_thread_mdelay(50);
    }
}

/* 呼吸灯效果 */
static void ws2812b_breathing(void)
{
    for (uint8_t brightness = 0; brightness < 255; brightness += 5)
    {
        ws2812b_set_brightness(brightness);
        ws2812b_show();
        rt_thread_mdelay(20);
    }

    for (uint8_t brightness = 255; brightness > 0; brightness -= 5)
    {
        ws2812b_set_brightness(brightness);
        ws2812b_show();
        rt_thread_mdelay(20);
    }
}

/* demo启动函数 */
static void ws2812b_demo_start(void)
{
    /* 确保 ws2812b_init 只执行一次 */
    static rt_bool_t initialized = RT_FALSE;

    if (!initialized)
    {
        ws2812b_init();
        initialized = RT_TRUE;
    }

    /* 亮度渐变 */
    ws2812b_brightness();

    /* 全部灯切换颜色 */
    ws2812b_full_color();

    /* 流水灯效果 */
    ws2812b_running();

    /* 呼吸灯效果 */
    ws2812b_breathing();
}

/* msh命令：启动WS2812B Demo */
static int cmd_ws2812b_demo(int argc, char **argv)
{
    ws2812b_demo_start();
    return RT_EOK;
}
MSH_CMD_EXPORT(cmd_ws2812b_demo, start WS2812B demo);