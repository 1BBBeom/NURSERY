/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     HUAWEI       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "yolo_model.h"  // 模型头文件（由转换工具生成）

/* 模型输入输出配置 */
#define INPUT_WIDTH      160
#define INPUT_HEIGHT     160
#define INPUT_CHANNELS   3
#define OUTPUT_SIZE      25200  // YOLOv5-tiny输出维度

/* Tensor Arena 内存池 */
static uint8_t tensor_arena[10*1024] RT_SECTION(".sram4");

void yolov5_thread_entry(void *param) {
    /* 1. 加载模型 */
    const tflite::Model* model = tflite::GetModel(g_yolov5_tflite);
    static tflite::MicroInterpreter static_interpreter;

    /* 2. 初始化解析器 */
    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddReshape();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();

    /* 3. 创建解释器 */
    tflite::MicroInterpreter interpreter(
        model, resolver, tensor_arena, sizeof(tensor_arena)
    );
    interpreter.AllocateTensors();

    /* 4. 获取输入输出张量 */
    TfLiteTensor* input = interpreter.input(0);
    TfLiteTensor* output = interpreter.output(0);

    while (1) {
        /* 5. 获取摄像头数据（伪代码） */
        uint8_t* camera_data = camera_capture(INPUT_WIDTH, INPUT_HEIGHT);

        /* 6. 数据预处理 */
        memcpy(input->data.uint8, camera_data, INPUT_WIDTH*INPUT_HEIGHT*3);

        /* 7. 执行推理 */
        interpreter.Invoke();

        /* 8. 解析YOLO输出 */
        float* predictions = output->data.f;
        for (int i = 0; i < OUTPUT_SIZE; i += 85) {
            float conf = predictions[i+4];
            if (conf > 0.5) {
                int class_id = argmax(&predictions[i+5], 80);
                rt_kprintf("Detected: %s (%.2f%%)\n",
                    class_names[class_id], conf*100);
            }
        }
        rt_thread_mdelay(100);
    }
}

int yolov5_init(void) {
    rt_thread_t tid = rt_thread_create(
        "yolo",
        yolov5_thread_entry,
        RT_NULL,
        4096,
        20,
        10
    );
    rt_thread_startup(tid);
    return 0;
}
INIT_APP_EXPORT(yolov5_init);
