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
#define ADC_DEV_CHANNEL     2
#define REFER_VOLTAGE       330
#define CONVERT_BITS        (1 << 16)

#define DRY_VOLTAGE    2.5f
#define WET_VOLTAGE    1.0f

void adc1_2_entry(void * parameters)
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
        rt_thread_mdelay(2000);
        adc_value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);

        // 计算电压
        voltage = (adc_value * REFER_VOLTAGE / CONVERT_BITS) * 0.01f;

        // 计算湿度
        float humidity = 100.0f * (DRY_VOLTAGE - voltage) / (DRY_VOLTAGE - WET_VOLTAGE);
        if (humidity > 100.0f) humidity = 100.0f;
        if (humidity < 0.0f) humidity = 0.0f;

        rt_kprintf("Soil ADC Raw: %d, Humidity: %.1f%%\n", adc_value, humidity);

        // 创建消息
        sensor_msg_t *msg = rt_malloc(sizeof(sensor_msg_t));
        if (!msg) {
            rt_kprintf("malloc for soil msg failed\n");
            continue;
        }

        msg->timestamp = rt_tick_get();
        msg->sensor_id = HUMI_EARTH;
        msg->value = humidity;

        // 发送消息
        rt_mutex_take(sensor_msg_mutex, RT_WAITING_FOREVER);
        rt_sem_take(sensor_msg_sem_empty, RT_WAITING_FOREVER);

        result = rt_mq_send(sensor_msg_mq, &msg, sizeof(sensor_msg_t*));

        if (result != RT_EOK) {
            rt_kprintf("rt_mq_send soil error: %d\n", result);
            rt_free(msg);
        }

        rt_mutex_release(sensor_msg_mutex);
    }
}

static int adc_read_volt_sample(void)
{
    rt_thread_t rt_thread_adc1_2 = rt_thread_create("soil_humidity",
                                                 adc1_2_entry,
                                                 RT_NULL,
                                                 THREAD_STACK_SIZE,
                                                 RT_THREAD_PRIORITY_MAX/2 + 2,
                                                 20);
    if (rt_thread_adc1_2 != RT_NULL) {
        rt_thread_startup(rt_thread_adc1_2);
        rt_kprintf("Soil humidity monitoring started!\n");
        return RT_EOK;
    }
    return -RT_ERROR;
}
INIT_APP_EXPORT(adc_read_volt_sample);


