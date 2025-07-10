/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     HUAWEI       the first version
 */
#ifndef APPLICATIONS_CONTROL_H_
#define APPLICATIONS_CONTROL_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "sensor_msg.h"

// PWM设备配置
#define FAN_PWM_DEVICE     "pwm3"
#define FAN_PWM_CHANNEL    2
#define FAN_PERIOD         10000000    // 10ms周期 (100Hz)

#define SERVO_PWM_DEVICE   "pwm3"
#define SERVO_PWM_CHANNEL  3
#define SERVO_PERIOD       20000000    // 20ms周期 (50Hz)
#define SERVO_MIN_PULSE    500000      // 0.5ms (0°)
#define SERVO_MAX_PULSE    2500000     // 2.5ms (180°)

// 水泵控制引脚
#define WATER_PUMP_PIN     GET_PIN(H, 3)

// 控制命令定义
#define CMD_MANUAL_ENABLE  200
#define CMD_MANUAL_DISABLE 201
#define PUMP_CTRL_BASE     300         // 水泵控制命令基准值

// 传感器数据结构
typedef struct {
    float temperature;     // 温度 (°C)
    float humidity;        // 空气湿度 (%)
    float soil_humidity;   // 土壤湿度 (%)
    float light_intensity; // 光照强度 (Lux)
} sensor_data_t;

// 全局变量声明
extern sensor_data_t sensor_data;
extern uint8_t g_manual_ctrl;

// 函数声明
void control_center_entry(void *parameters);
int control_center_init(void);

#endif /* APPLICATIONS_CONTROL_H_ */
