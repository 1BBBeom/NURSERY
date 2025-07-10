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
#include <sys/socket.h>
#include <arpa/inet.h>
#include "board.h"
#include "sensor_msg.h"
#include "sean_ws2812b.h"
#include <wlan_mgnt.h>
#include <wlan_cfg.h>
#include <drv_sdio.h>

//硬件配置
#define FAN_PWM_DEVICE     "pwm3"
#define FAN_PWM_CHANNEL    2
#define FAN_PERIOD         10000000  //10ms周期
#define SERVO_PWM_DEVICE   "pwm3"
#define SERVO_PWM_CHANNEL  3
#define SERVO_PERIOD       20000000  //20ms周期
#define SERVO_MIN_PULSE    500000    //0.5ms (0°)
#define SERVO_MAX_PULSE    2500000   //2.5ms (180°)
#define WATER_PUMP_PIN     GET_PIN(H, 3)




#define AP_SSID              "NURSERY"
#define AP_PASSWORD          "12345678"
#define AP_CHANNEL           6

#define HTTP_PORT            80
#define MAX_CONNECTIONS      5
#define RECV_BUF_SIZE        1024


//控制命令定义
#define CMD_MANUAL_ENABLE  200
#define CMD_MANUAL_DISABLE 201
#define PUMP_CTRL_BASE     300

//全局变量
static struct rt_device_pwm *pwm_fan;
static struct rt_device_pwm *pwm_servo;
static rt_tick_t pump_start_time = 0;
uint8_t g_manual_ctrl = CMD_MANUAL_DISABLE;

//传感器数据结构
typedef struct {
    float temperature;
    float humidity;
    float soil_humidity;
    float light_intensity;
} sensor_data_t;

static sensor_data_t sensor_data = {0};
static struct rt_wlan_info ap_info;

//网页HTML内容
const char *index_html =
    "<html><head><title>智能农业控制系统</title></head>"
    "<body><h1>欢迎使用智能农业控制系统</h1>"
    "<p><a href='/dashboard.html'>仪表盘</a></p>"
    "<p><a href='/control.html'>设备控制</a></p></body></html>";

const char *dashboard_html =
    "<html><head><title>仪表盘</title><meta http-equiv=\"refresh\" content=\"2\">"
    "<style>body{font-family:Arial,sans-serif;margin:20px}</style></head>"
    "<body><h1>环境数据监测</h1>"
    "<div id='sensor-data'>加载中...</div>"
    "<script>"
    "function updateData(){"
    "  fetch('/api/sensors').then(r=>r.json()).then(data=>{"
    "    document.getElementById('sensor-data').innerHTML = "
    "      `温度: ${data.temp}°C<br>湿度: ${data.humi}%<br>"
    "      土壤湿度: ${data.soil}%<br>光照: ${data.light}Lux`;"
    "  });"
    "}"
    "setInterval(updateData, 2000);"
    "updateData();"
    "</script></body></html>";

const char *control_html =
    "<html><head><title>设备控制</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:20px}"
    ".control-group{margin:15px 0}"
    "input[type=range]{width:300px}"
    "</style></head>"
    "<body><h1>设备控制面板</h1>"

    "<div class='control-group'>"
    "<label>风扇速度: <span id='fan-value'>0</span>%</label><br>"
    "<input type='range' min='0' max='100' value='0' id='fan-slider'>"
    "</div>"

    "<div class='control-group'>"
    "<label>舵机角度: <span id='servo-value'>0</span>°</label><br>"
    "<input type='range' min='0' max='180' value='90' id='servo-slider'>"
    "</div>"

    "<div class='control-group'>"
    "<label>水泵控制: </label>"
    "<button id='pump-on'>开启</button>"
    "<button id='pump-off'>关闭</button>"
    "</div>"

    "<script>"
    "// 风扇控制"
    "document.getElementById('fan-slider').oninput = function(){"
    "  document.getElementById('fan-value').textContent = this.value;"
    "  fetch('/api/control?cmd=' + this.value);"
    "};"

    "// 舵机控制"
    "document.getElementById('servo-slider').oninput = function(){"
    "  var angle = parseInt(this.value);"
    "  document.getElementById('servo-value').textContent = angle;"
    "  fetch('/api/control?cmd=' + (103 + angle));"  // 103-283对应0-180°
    "};"

    "// 水泵控制"
    "document.getElementById('pump-on').onclick = function(){"
    "  fetch('/api/control?cmd=301');"  // 开启水泵
    "};"
    "document.getElementById('pump-off').onclick = function(){"
    "  fetch('/api/control?cmd=300');"  // 关闭水泵
    "};"
    "</script></body></html>";

//启动AP模式
int start_ap_mode(void)
{
    rt_err_t result = RT_EOK;

    /*配置AP参数 */
    rt_memset(&ap_info, 0, sizeof(struct rt_wlan_info));
    SSID_SET(&ap_info, AP_SSID);
    ap_info.channel = AP_CHANNEL;
    ap_info.security = SECURITY_WPA2_AES_PSK;

    /*设置WLAN模式为AP */
    result = rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);
    if (result != RT_EOK)
    {
        rt_kprintf("设置WLAN模式为AP失败！\n");
        return -RT_ERROR;
    }

    /*启动AP*/
    result = rt_wlan_start_ap_adv(&ap_info, AP_PASSWORD);
    if (result != RT_EOK)
    {
        rt_kprintf("启动AP失败！\n");
        return -RT_ERROR;
    }

    rt_kprintf("AP已启动，SSID: %s, 密码: %s\n", AP_SSID, AP_PASSWORD);

    /*启动DHCP服务器 */
    extern void dhcpd_start(const char *netif_name);
    dhcpd_start("w0");

    return RT_EOK;
}

//控制命令处理函数
static void handle_control_command(rt_uint32_t cmd_value)
{
    rt_kprintf("收到控制命令: %d\n", cmd_value);

    //手动模式切换
    if (cmd_value == CMD_MANUAL_DISABLE || cmd_value == CMD_MANUAL_ENABLE) {
        g_manual_ctrl = cmd_value;
        rt_kprintf("手动控制 %s\n",
                  (cmd_value == CMD_MANUAL_ENABLE) ? "已启用" : "已禁用");
        return;
    }

    //仅在手动模式下执行设备控制
    if (g_manual_ctrl == CMD_MANUAL_ENABLE) {
        //风扇控制 (0-100)
        if (cmd_value <= 100) {
            rt_uint32_t fan_pulse = cmd_value * FAN_PERIOD / 100;
            rt_pwm_set(pwm_fan, FAN_PWM_CHANNEL, FAN_PERIOD, fan_pulse);
            rt_kprintf("风扇速度设置为: %d%%\n", cmd_value);
        }
        //舵机控制 (103-282)
        else if (cmd_value >= 103 && cmd_value <= 282) {
            rt_uint32_t angle = cmd_value - 103;
            rt_uint32_t pulse = SERVO_MIN_PULSE +
                angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 180;
            rt_pwm_set(pwm_servo, SERVO_PWM_CHANNEL, SERVO_PERIOD, pulse);
            rt_kprintf("舵机角度设置为: %d°\n", angle);
        }
        //水泵控制
        else if (cmd_value >= PUMP_CTRL_BASE) {
            rt_uint8_t state = cmd_value - PUMP_CTRL_BASE;
            rt_pin_write(WATER_PUMP_PIN, state ? PIN_HIGH : PIN_LOW);
            if (state) {
                pump_start_time = rt_tick_get();
                rt_kprintf("水泵已启动\n");
            } else {
                rt_kprintf("水泵已停止\n");
            }
        }
    }
}

void auto_thread()
{
    while(1) {
        //传感器数据采集（温/光/湿）
        env_data = read_sensors();
        //AI生长阶段识别（YOLOv5）
        growth_stage = yolov5_detect();
        //生成控制指令（如：幼苗期补蓝光）
        cmd = ai_decision(env_data, growth_stage);
        //执行调控（参见控制命令处理函数）
        execute_control(cmd);
        rt_thread_mdelay(1000);  //1s决策周期
    }
}

//HTTP服务器线程
static void http_server_thread(void *parameter)
{
    int sock, connected;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char recv_data[1024];
    int is_running = 1;

    //创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        rt_kprintf("Socket创建失败\n");
        return;
    }

    //配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //绑定socket
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != RT_EOK)
    {
        rt_kprintf("绑定端口80失败\n");
        closesocket(sock);
        return;
    }

    //开始监听
    listen(sock, 5);
    rt_kprintf("HTTP服务器已启动，监听端口80\n");

    while (is_running) {
        //接受客户端连接
        if ((connected = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            rt_kprintf("接受连接失败\n");
            continue;
        }

        //接收客户端数据
        int recv_len = recv(connected, recv_data, sizeof(recv_data) - 1, 0);
        if (recv_len <= 0) {
            closesocket(connected);
            continue;
        }
        recv_data[recv_len] = '\0';

        //解析请求类型和路径
        char *method = strtok(recv_data, " ");
        char *path = strtok(NULL, " ");

        if (!method || !path) {
            closesocket(connected);
            continue;
        }

        //处理API请求
        if (strcmp(path, "/api/sensors") == 0) {
            char json[128];
            rt_snprintf(json, sizeof(json),
                "{\"temp\":%.1f,\"humi\":%.1f,\"soil\":%.1f,\"light\":%.1f}",
                sensor_data.temperature,
                sensor_data.humidity,
                sensor_data.soil_humidity,
                sensor_data.light_intensity);

            char header[256];
            rt_snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n\r\n",
                rt_strlen(json));

            send(connected, header, rt_strlen(header), 0);
            send(connected, json, rt_strlen(json), 0);
        }
        //处理控制命令
        else if (strcmp(path, "/api/control") == 0) {
            char *cmd_str = strstr(recv_data, "cmd=");
            if (cmd_str) {
                handle_control_command(atoi(cmd_str + 4));
                send(connected, "HTTP/1.1 200 OK\r\n\r\nOK", 22, 0);
            } else {
                send(connected, "HTTP/1.1 400 Bad Request\r\n\r\nError", 33, 0);
            }
        }
        //处理网页请求
        else if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
            char header[256];
            rt_snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n",
                rt_strlen(index_html));

            send(connected, header, rt_strlen(header), 0);
            send(connected, index_html, rt_strlen(index_html), 0);
        }
        else if (strcmp(path, "/dashboard.html") == 0) {
            char header[256];
            rt_snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n",
                rt_strlen(dashboard_html));

            send(connected, header, rt_strlen(header), 0);
            send(connected, dashboard_html, rt_strlen(dashboard_html), 0);
        }
        else if (strcmp(path, "/control.html") == 0) {
            char header[256];
            rt_snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n",
                rt_strlen(control_html));

            send(connected, header, rt_strlen(header), 0);
            send(connected, control_html, rt_strlen(control_html), 0);
        }
        //处理其他请求
        else {
            send(connected, "HTTP/1.1 404 Not Found\r\n\r\nNot Found", 36, 0);
        }

        closesocket(connected);
    }

    closesocket(sock);
}

//主控制线程
void control_center_entry(void *parameters)
{
    //初始化风扇PWM
    pwm_fan = (struct rt_device_pwm *)rt_device_find(FAN_PWM_DEVICE);
    if (pwm_fan == RT_NULL) {
        rt_kprintf("风扇PWM设备 %s 未找到!\n", FAN_PWM_DEVICE);
    } else {
        rt_pwm_set(pwm_fan, FAN_PWM_CHANNEL, FAN_PERIOD, 0);
        rt_pwm_enable(pwm_fan, FAN_PWM_CHANNEL);
    }

    //初始化舵机PWM
    pwm_servo = (struct rt_device_pwm *)rt_device_find(SERVO_PWM_DEVICE);
    if (pwm_servo == RT_NULL) {
        rt_kprintf("舵机PWM设备 %s 未找到!\n", SERVO_PWM_DEVICE);
    } else {
        rt_pwm_set(pwm_servo, SERVO_PWM_CHANNEL, SERVO_PERIOD, 1500000);
        rt_pwm_enable(pwm_servo, SERVO_PWM_CHANNEL);
    }

    //初始化水泵GPIO
    rt_pin_mode(WATER_PUMP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(WATER_PUMP_PIN, PIN_LOW);

    //创建HTTP服务器线程
    rt_thread_t http_thread = rt_thread_create("http_server",
                                             http_server_thread,
                                             RT_NULL,
                                             2048,
                                             10,
                                             10);
    if (http_thread) {
        rt_thread_startup(http_thread);
        rt_kprintf("HTTP服务器线程已启动\n");
    }

    //主循环 - 处理传感器数据和控制逻辑
    while (1) {
        sensor_msg_t *msg_ptr = RT_NULL;

        //从消息队列接收传感器数据指针
        if (rt_mq_recv(sensor_msg_mq, &msg_ptr, sizeof(msg_ptr), RT_WAITING_FOREVER) == RT_EOK) {
            if (msg_ptr) {
                // 更新全局传感器数据
                switch (msg_ptr->sensor_id) {
                case TEMP_INSIDE:
                    sensor_data.temperature = msg_ptr->value;
                    break;
                case HUMI_INSIDE:
                    sensor_data.humidity = msg_ptr->value;
                    break;
                case HUMI_EARTH:
                    sensor_data.soil_humidity = msg_ptr->value;
                    break;
                case LIGHT_OUTSIDE:
                    sensor_data.light_intensity = msg_ptr->value;
                    break;
                default:
                    break;
                }

                rt_kprintf("传感器更新: T=%.1fC H=%.1f%% S=%.1f%% L=%.1fLux\n",
                          sensor_data.temperature,
                          sensor_data.humidity,
                          sensor_data.soil_humidity,
                          sensor_data.light_intensity);

                // 释放消息内存
                rt_free(msg_ptr);
            }

            // 释放信号量
            rt_sem_release(sensor_msg_sem_empty);
        }

        // 自动停止水泵(运行超过5秒)
        if (rt_pin_read(WATER_PUMP_PIN) &&
            rt_tick_get() - pump_start_time > rt_tick_from_millisecond(5000))
        {
            rt_pin_write(WATER_PUMP_PIN, PIN_LOW);
            rt_kprintf("水泵自动关闭\n");
        }

        rt_thread_mdelay(100);
    }
}

// 初始化函数
static int control_center_init(void)
{
    rt_thread_t control_thread = rt_thread_create("control_center",
                                               control_center_entry,
                                               RT_NULL,
                                               4096,
                                               8,
                                               10);
    if (control_thread) {
        rt_thread_startup(control_thread);
        rt_kprintf("控制中心已启动\n");
        return RT_EOK;
    }
    return -RT_ERROR;
}
INIT_APP_EXPORT(control_center_init);
