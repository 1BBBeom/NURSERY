/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     HUAWEI       the first version
 */


#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "sensor.h"
#include "sensor_msg.h"

rt_mq_t sensor_msg_mq;
rt_mutex_t sensor_msg_mutex;
rt_sem_t sensor_msg_sem_empty;
rt_mailbox_t sensor_msg_mb;

char *const g_sensor_name_str[] = {
    "light_outside",
    "temp_inside",
    "humi_inside",
    "humi_earth",
    "fan_top",
    "water_pump",
};

rt_err_t sensor_msg_mq_creat(void)
{
    int result = RT_EOK;

    // 创建消息队列
    sensor_msg_mq = rt_mq_create("sensor_mq",
                                sizeof(sensor_msg_t*), // 存储指针的大小
                                16, // 队列长度
                                RT_IPC_FLAG_FIFO);
    if (sensor_msg_mq == RT_NULL) {
        rt_kprintf("create sensor_msg_mq failed\n");
        result = -RT_ERROR;
        goto __exit;
    }

    // 创建互斥锁
    sensor_msg_mutex = rt_mutex_create("sensor_mutex", RT_IPC_FLAG_FIFO);
    if (sensor_msg_mutex == RT_NULL) {
        rt_kprintf("create sensor_msg_mutex failed\n");
        result = -RT_ERROR;
        goto __cleanup_mq;
    }

    // 创建信号量
    sensor_msg_sem_empty = rt_sem_create("msg_empty", 10, RT_IPC_FLAG_FIFO);
    if (sensor_msg_sem_empty == RT_NULL) {
        rt_kprintf("create sensor_msg_sem_empty failed\n");
        result = -RT_ERROR;
        goto __cleanup_mutex;
    }

    // 创建邮箱
    sensor_msg_mb = rt_mb_create("sensor_mb", 16, RT_IPC_FLAG_FIFO);
    if (sensor_msg_mb == RT_NULL) {
        rt_kprintf("create sensor_msg_mb failed\n");
        result = -RT_ERROR;
        goto __cleanup_sem;
    }

    return RT_EOK;

__cleanup_sem:
    rt_sem_delete(sensor_msg_sem_empty);
__cleanup_mutex:
    rt_mutex_delete(sensor_msg_mutex);
__cleanup_mq:
    rt_mq_delete(sensor_msg_mq);
__exit:
    return result;
}
INIT_COMPONENT_EXPORT(sensor_msg_mq_creat);

//
//#include <rtthread.h>第一版本
//#include <rtdevice.h>
//#include "board.h"
//#include "sensor.h"
//#include "sensor_msg.h"
//
//rt_mq_t sensor_msg_mq;
//rt_mutex_t sensor_msg_mutex;
//rt_sem_t sensor_msg_sem_empty;
//rt_mailbox_t sensor_msg_mb;
////uint8_t g_manual_ctrl = 101;
//
//char *const g_sensor_name_str[] =
//{
//        "light_outside",
//        "temp_inside",
//        "humi_inside",
//        "temp_earth",
//        "humi_earth",
//        "fan_top",
//        "water_pump",
////        "manual_ctrl",
//};
//
//
//
//rt_err_t sensor_msg_mq_creat(void)
//{
//    sensor_msg_t sensor_msg;
//    int result = RT_EOK;
//
//
//    sensor_msg_mq = rt_mq_create("sensor_mq", MQ_BLOCK_SIZE, MQ_LEN, RT_IPC_FLAG_FIFO);
//    if (sensor_msg_mq == RT_NULL)
//    {
//        rt_kprintf("init  sensor_msg_mq failed.\n");
//        result = -RT_ERROR;
//        goto __exit;
//
//    }
//
//
//    sensor_msg_mutex = rt_mutex_create("sensor_dmutex", RT_IPC_FLAG_FIFO);
//    if (sensor_msg_mutex == RT_NULL)
//    {
//        rt_kprintf("create sensor_msg_mutex failed.\n");
//        result = -RT_ERROR;
//        goto __cleanup_mq;
//    }
//
//
//    sensor_msg_sem_empty = rt_sem_create("msg_empty", 3, RT_IPC_FLAG_FIFO);
//    if (sensor_msg_sem_empty == RT_NULL)
//    {
//        rt_kprintf("create dynamic semaphore failed.\n");
////      return -1;
//        result = -RT_ERROR;
//        goto __cleanup_mutex;
//    }
//
//
//    sensor_msg_mb = rt_mb_create("sensor_mb", 16, RT_IPC_FLAG_FIFO);
//    if (sensor_msg_mb == RT_NULL)
//    {
//        rt_kprintf("create dynamic mailbox failed.\n");
////        return -1;
//        result = -RT_ERROR;
//        goto __cleanup_sem;
//    }
//
//
//
//    return RT_EOK;
//
//
//    __cleanup_sem:
//        rt_sem_delete(sensor_msg_sem_empty);
//
//    __cleanup_mutex:
//        rt_mutex_delete(sensor_msg_mutex);
//
//    __cleanup_mq:
//        rt_mq_delete(sensor_msg_mq);
//
//    __exit:
//        return result;
//}
//INIT_COMPONENT_EXPORT(sensor_msg_mq_creat);
//
