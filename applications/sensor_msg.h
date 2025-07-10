/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     HUAWEI       the first version
 */
#ifndef APPLICATIONS_SENSOR_MSG_H_
#define APPLICATIONS_SENSOR_MSG_H_


#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include "sensor.h"

#define MQ_BLOCK_SIZE       RT_ALIGN(sizeof(sensor_msg_t), sizeof(intptr_t)) /* 涓轰簡瀛楄妭瀵归綈 */
#define MQ_LEN              (6)

extern uint8_t g_led_brightness;

typedef enum sensor_id_
{
   LIGHT_OUTSIDE = 0,
   TEMP_INSIDE,
   HUMI_INSIDE,
   HUMI_EARTH,
   FNA_TOP,
   WATER_PUMP,
   MANUAL_CTRL,
}sensor_id_t;

typedef enum control_id_
{
//   FNA_TOP = 0,
   RRLY_WATER,
}control_id_t;

typedef struct sensor_msg_
{
    rt_tick_t timestamp;
    sensor_id_t sensor_id;
    float value;
}sensor_msg_t;


extern rt_mq_t sensor_msg_mq;

extern rt_mutex_t sensor_msg_mutex;

extern rt_sem_t sensor_msg_sem_empty;

extern char *const g_sensor_name_str[];

extern uint8_t g_manual_ctrl;

extern rt_mailbox_t sensor_msg_mb;

rt_err_t sensor_msg_mq_creat(void);


#endif /* APPLICATIONS_SENSOR_MSG_H_ */

