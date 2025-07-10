
# sean_ws2812b

**基于SPI接口的WS2812B灯带驱动软件包，适用于RT-Thread 4.1.0系统。**

---

## 📚 软件包简介

`sean_ws2812b` 是一款基于 SPI 的 WS2812B 灯带驱动软件包，基于 RT-Thread SPI框架 设计。  
通过 SPI 高速输出编码数据，实现流畅、稳定的多彩灯效展示。  

本软件包体积小巧，接口简单，功能完整，适合嵌入各种 RGB 灯控项目中。

---

## ✨ 主要功能特性

- 📌 支持 SPI 接口驱动 WS2812B 灯带  
- 📌 支持单颗灯珠独立控制颜色  
- 📌 支持批量设置所有灯珠统一颜色  
- 📌 支持动态调整全局亮度（0~255）  
- 📌 内置 Gamma 校正，显示更自然  
- 📌 提供多种灯效示例（如流水灯、呼吸灯）  
- 📌 支持通过 Kconfig 自定义 SPI总线、设备名称、片选引脚、灯珠数量  

---

## 🚀 软件包快速使用

### 1. 引入软件包

在 RT-Thread Env 或 Studio 的 `menuconfig` 中，选择：

```
RT-Thread online packages → Peripheral libraries and drivers → sean_ws2812b
```

✔ 勾选启用，保存配置，自动下载源码到 `packages/sean_ws2812b-latest/` 目录。

---

### 2. 配置参数（可选）

在 `menuconfig` 中可进一步配置：

| 配置项 | 默认值 | 说明 |
|:--|:--|:--|
| SEAN_WS2812B_SPI_BUS_NAME | "spi1" | SPI总线控制器名称 |
| SEAN_WS2812B_SPI_DEV_NAME | "spi10" | SPI挂载设备名称 |
| SEAN_WS2812B_SPI_CS_PORT | "GPIOB" | 片选GPIO端口 |
| SEAN_WS2812B_SPI_CS_PIN | 12 | 片选GPIO引脚号 |
| SEAN_WS2812B_LED_NUMS | 5 | 灯带灯珠数量 |
| SEAN_WS2812B_USING_DEMO | 关闭 | 是否编译示例代码（demo） |

---

### 3. 基本使用示例

#### 初始化灯带

```c
#include <sean_ws2812b.h>

int main(void)
{
    ws2812b_init(); // 初始化SPI及灯带

    ws2812b_set_all(SEAN_WS2812B_COLOR_GREEN); // 设置全部灯珠为绿色
    ws2812b_show(); // 刷新显示

    return 0;
}
```

#### 设置单个灯珠颜色

```c
ws2812b_set_color(3, SEAN_WS2812B_COLOR_RED); // 将第4颗灯珠设置为红色
ws2812b_show();
```

#### 调整整体亮度

```c
ws2812b_set_brightness(128); // 全局亮度调整为一半
ws2812b_show();
```

---

### 4. 使用 Demo 示例

如果在 Kconfig 中启用了 `Enable demo examples`：

可直接在 MSH 命令行中运行示例程序：

```bash
msh > ws2812b_demo
```

该示例包括：

- 亮度渐变演示  
- 流水灯效果  
- 呼吸灯效果  

---

## 📎 注意事项

- ⚡ 请确保你的 SPI 总线速度设置为 6MHz 左右，以满足 WS2812B 灯带时序要求。
- ⚡ 配置步骤说明：
  1. **在RT-Thread `menuconfig`中，打开 SPI 驱动（RT_USING_SPI）**。
  2. **在工程的 `board.h` 文件中，取消注释所需的 SPI 总线宏定义，例如：**
     ```c
     #define BSP_USING_SPI1
     ```
  3. **在 STM32CubeMX 工具中配置对应的SPI接口：**
     - 仅启用 **半双工单向发送（Transmit Only Half Duplex）** 模式。
     - 配置为主机模式（Master）且设置合理的波特率（建议控制在6MHz左右）。
- ⚡ 实测结果：
  - STM32F103 系列，主频48MHz环境下，灯效显示正常。
  - STM32H743 系列，主频68MHz环境下，灯效显示正常。
- ⚡ **如果出现灯带无法点亮、闪烁异常等问题，请优先检查主控芯片主频设置是否合理。** 不同主频可能导致SPI时序偏差，需要保证时序符合WS2812B要求。
- ⚡ SPI通信需要注意片选引脚（CS）是否正确配置。
- ⚡ 大量灯珠（如>100颗）时，需要适当控制刷新频率，以避免阻塞系统任务。

---

## 📋 TODO清单

- [ ] 增加基于PWM方式的WS2812B驱动支持  
- [x] 适配 RT-Thread 5.1.0 及以上版本 ✅

---

## 📄 许可协议

本项目遵循 [**Apache-2.0 License**](LICENSE)。

---

## 🛠 版本历史

| 版本 | 更新内容 |
|:--|:--|
| v1.0.0 | 发布初版，实现基本功能，支持SPI驱动、亮度控制、灯效demo演示。 |
| v1.1.0 | 适配 RT-Thread 5.1.0 及以上版本。 |

---

## 👨‍💻 作者信息

- **作者**：godmial  
- **邮箱**：2633967641@qq.com  
- **Gitee**：[godmial](https://gitee.com/godmial)
