/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     HUAWEI       the first version
 */



#include <rtthread.h>第二版本
#include <rtdevice.h>
#include "sensor.h"
#include "sensor_dallas_dht11.h"
#include "drv_gpio.h"
#include "sensor_msg.h"

#define DHT11_DATA_PIN    GET_PIN(H, 2)

static void read_temp_entry(void *parameter)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    int result;
    rt_uint8_t get_data_freq = 1; /* 1Hz */

    dev = rt_device_find("temp_dht11");
    if (dev == RT_NULL) {
        rt_kprintf("DHT11 device not found!\n");
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK) {
        rt_kprintf("open DHT11 device failed!\n");
        return;
    }

    rt_device_control(dev, RT_SENSOR_CTRL_SET_ODR, (void *)(&get_data_freq));

    while (1) {
        rt_size_t res = rt_device_read(dev, 0, &sensor_data, 1);
        if (res != 1) {
            rt_kprintf("read DHT11 data failed! result: %d\n", res);
            rt_thread_mdelay(2000);
            continue;
        }

        if (sensor_data.data.temp >= 0) {
            // 创建两个独立的消息
            sensor_msg_t *temp_msg = rt_malloc(sizeof(sensor_msg_t));
            sensor_msg_t *humi_msg = rt_malloc(sizeof(sensor_msg_t));

            if (!temp_msg || !humi_msg) {
                rt_kprintf("malloc for sensor msg failed\n");
                if (temp_msg) rt_free(temp_msg);
                if (humi_msg) rt_free(humi_msg);
                continue;
            }

            // 填充温度消息
            temp_msg->timestamp = rt_tick_get();
            temp_msg->sensor_id = TEMP_INSIDE;
            temp_msg->value = (sensor_data.data.temp & 0xffff) >> 0;

            // 填充湿度消息
            humi_msg->timestamp = rt_tick_get();
            humi_msg->sensor_id = HUMI_INSIDE;
            humi_msg->value = (sensor_data.data.temp & 0xffff0000) >> 16;

            // 发送消息
            rt_mutex_take(sensor_msg_mutex, RT_WAITING_FOREVER);

            rt_sem_take(sensor_msg_sem_empty, RT_WAITING_FOREVER);
            result = rt_mq_send(sensor_msg_mq, &temp_msg, sizeof(sensor_msg_t*));
            if (result != RT_EOK) {
                rt_kprintf("rt_mq_send TEMP_INSIDE ERR\n");
            }

            rt_sem_take(sensor_msg_sem_empty, RT_WAITING_FOREVER);
            result = rt_mq_send(sensor_msg_mq, &humi_msg, sizeof(sensor_msg_t*));
            if (result != RT_EOK) {
                rt_kprintf("rt_mq_send HUMI_INSIDE ERR\n");
            }

            rt_mutex_release(sensor_msg_mutex);
        }

        rt_thread_mdelay(2000);
    }
}

static int dht11_read_temp_sample(void)
{
    rt_thread_t dht11_thread = rt_thread_create("dht_tem",
                                              read_temp_entry,
                                              RT_NULL,
                                              1024,
                                              RT_THREAD_PRIORITY_MAX/2 + 1,
                                              20);
    if (dht11_thread != RT_NULL) {
        rt_thread_startup(dht11_thread);
        return RT_EOK;
    }
    return -RT_ERROR;
}
INIT_APP_EXPORT(dht11_read_temp_sample);

static int rt_hw_dht11_port(void)
{
    struct rt_sensor_config cfg;
    cfg.intf.user_data = (void *)DHT11_DATA_PIN;
    rt_hw_dht11_init("dht11", &cfg);
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_dht11_port);

