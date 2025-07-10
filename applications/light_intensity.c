/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     HUAWEI       the first version
 */

#include <stdlib.h>第二版本
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "sensor_msg.h"

#define THREAD_STACK_SIZE 512
#define ADC_DEV_NAME        "adc1"
#define ADC_DEV_CHANNEL     3
#define REFER_VOLTAGE       330
#define CONVERT_BITS        (1 << 16)

#define DARK_VOLTAGE     0.1f
#define BRIGHT_VOLTAGE   2.0f
#define MAX_LUX          2000.0f

void adc_light_entry(void * parameters)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t adc_value;
    float voltage;
    int result;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if(adc_dev == RT_NULL) {
        rt_kprintf("ADC device %s not found!\n", ADC_DEV_NAME);
        return;
    }

    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);

    while(1) {
        rt_thread_mdelay(1000);
        adc_value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);

        // 计算电压
        voltage = (adc_value * REFER_VOLTAGE / CONVERT_BITS) * 0.01f;

        // 计算光照强度
        float light_intensity;
        if (voltage <= DARK_VOLTAGE) {
            light_intensity = 0.0f;
        } else if (voltage >= BRIGHT_VOLTAGE) {
            light_intensity = MAX_LUX;
        } else {
            light_intensity = MAX_LUX * (voltage - DARK_VOLTAGE) / (BRIGHT_VOLTAGE - DARK_VOLTAGE);
        }

        rt_kprintf("Light ADC Raw: %d, Lux: %.1f\n", adc_value, light_intensity);

        // 创建消息
        sensor_msg_t *msg = rt_malloc(sizeof(sensor_msg_t));
        if (!msg) {
            rt_kprintf("malloc for light msg failed\n");
            continue;
        }

        msg->timestamp = rt_tick_get();
        msg->sensor_id = LIGHT_OUTSIDE;
        msg->value = light_intensity;

        // 发送消息
        rt_mutex_take(sensor_msg_mutex, RT_WAITING_FOREVER);
        rt_sem_take(sensor_msg_sem_empty, RT_WAITING_FOREVER);

        result = rt_mq_send(sensor_msg_mq, &msg, sizeof(sensor_msg_t*));

        if (result != RT_EOK) {
            rt_kprintf("rt_mq_send light error: %d\n", result);
            rt_free(msg);
        }

        rt_mutex_release(sensor_msg_mutex);
    }
}

static int light_sensor_sample(void)
{
    rt_thread_t rt_thread_light = rt_thread_create("light_sensor",
                                                adc_light_entry,
                                                RT_NULL,
                                                THREAD_STACK_SIZE,
                                                RT_THREAD_PRIORITY_MAX/4,
                                                20);
    if (rt_thread_light != RT_NULL) {
        rt_thread_startup(rt_thread_light);
        rt_kprintf("Light intensity monitoring started!\n");
        return RT_EOK;
    }
    return -RT_ERROR;
}
INIT_APP_EXPORT(light_sensor_sample);

