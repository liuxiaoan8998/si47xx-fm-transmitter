# Si47xx FM Transmitter - LCD 错误码系统

## 📋 快速参考

### 错误码速查表

| 错误码 | 名称 | 描述 | 解决方法 |
|--------|------|------|----------|
| **0** | OK | 系统正常 | 无需处理 |
| **101** | I2C-W | I2C 写失败 | 检查 SDA/SCL 连接 |
| **102** | I2C-R | I2C 读失败 | 检查上拉电阻 |
| **103** | I2C-T | I2C 超时 | 检查时钟频率 |
| **104** | I2C-N | I2C 无应答 | 检查设备地址 |
| **200** | CHIP-0 | 芯片未检测到 | 检查电源和复位 |
| **201** | CHIP-I | 初始化失败 | 检查配置参数 |
| **202** | CHIP-V | 版本不匹配 | 更新固件 |
| **203** | CHIP-R | 复位失败 | 检查复位电路 |
| **301** | F-LOW | 频率过低 | 调整到 87.5MHz 以上 |
| **302** | F-HIGH | 频率过高 | 调整到 108MHz 以下 |
| **303** | F-LOCK | 频率锁定失败 | 检查天线 |
| **304** | F-TUNE | 调谐失败 | 重试或重启 |
| **401** | P-LOW | 功率过低 | 检查功率设置 |
| **402** | P-HIGH | 功率过高 | 降低功率 |
| **403** | P-ADJ | 功率调整失败 | 检查 PA 电路 |
| **501** | A-MUTE | 音频静音 | 取消静音 |
| **502** | A-VOL | 音量设置失败 | 检查音频通路 |
| **503** | A-IN | 音频输入异常 | 检查输入信号 |
| **601** | R-ENA | RDS 使能失败 | 检查芯片支持 |
| **602** | R-DAT | RDS 数据错误 | 检查数据格式 |
| **603** | R-SYN | RDS 同步失败 | 等待同步 |
| **701** | S-MEM | 内存不足 | 释放内存 |
| **702** | S-STK | 栈溢出 | 优化代码 |
| **703** | S-WDG | 看门狗复位 | 检查程序卡死 |
| **704** | S-CLK | 时钟异常 | 检查晶振 |
| **801** | U-PAR | 参数错误 | 检查输入参数 |
| **802** | U-RNG | 超出范围 | 检查参数范围 |
| **803** | U-STA | 状态错误 | 检查调用顺序 |
| **999** | UKN | 未定义错误 | 查阅手册 |

---

## 🔧 使用方法

### 1. 硬件接口实现

在你的项目中实现以下函数：

```c
/* LCD 显示函数 */
void LCD_dispnum(char number)
{
    /* 你的 LCD 驱动代码 */
    /* 显示单个数字 0-9 */
}

void LCD_clear(void)
{
    /* 清除 LCD 显示 */
}

void delay_ms(uint32_t ms)
{
    /* 毫秒延时 */
}

/* I2C 接口 */
int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len)
{
    /* I2C 写操作 */
}

int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    /* I2C 读操作 */
}
```

### 2. 显示错误码

```c
#include "lcd_error_codes.h"

/* 简单显示 */
lcd_show_error(ERR_I2C_WRITE);  /* 显示 101 */

/* 闪烁显示 (引起注意) */
lcd_blink_error(ERR_CHIP_INIT, 3);  /* 闪烁 3 次 */

/* 清除错误 */
lcd_clear_error();  /* 显示 000 */
```

### 3. 获取错误详情

```c
const lcd_error_info_t *info = lcd_get_error_info(ERR_FREQ_LOW);

printf("错误：%s\r\n", info->name);
printf("描述：%s\r\n", info->desc);
printf("解决：%s\r\n", info->solution);
```

---

## 📁 文件结构

```
si47xx-fm-transmitter/
├── lcd_error_codes.h       # 错误码定义
├── lcd_error_display.c     # LCD 显示实现
├── test_with_lcd.c         # 测试程序
├── si47xx_fm_transmitter.h # Si47xx 驱动头文件
├── si47xx_fm_transmitter.c # Si47xx 驱动实现
├── Makefile                # 构建文件
└── README.md               # 主说明文档
```

---

## 🏗️ 编译

```bash
cd ~/Projects/si47xx-fm-transmitter

# 编译 LCD 测试程序
make lcd_test

# 或手动编译
gcc -o test_with_lcd test_with_lcd.c si47xx_fm_transmitter.c lcd_error_display.c
```

---

## 💡 错误码设计原则

### 分类编码

- **0xx**: 成功状态
- **1xx**: I2C 通信错误
- **2xx**: 芯片相关错误
- **3xx**: 频率相关错误
- **4xx**: 功率相关错误
- **5xx**: 音频相关错误
- **6xx**: RDS 相关错误
- **7xx**: 系统错误
- **8xx**: 用户错误
- **9xx**: 保留

### 3 位显示优化

由于 3 位断码屏限制：
- 直接显示数字 (000-999)
- 便于快速识别和记录
- 可配合本表格查阅含义

---

## 🔍 调试技巧

### 1. 开机自检

系统启动时会自动显示 `000`，表示正常启动。

### 2. 错误闪烁

重要错误会闪烁 3 次，然后保持显示。

### 3. 串口调试

启用串口输出查看详细错误信息：

```c
lcd_print_error_debug(ERR_I2C_WRITE);
```

输出：
```
=== 错误信息 ===
代码：101
名称：I2C-W
描述：I2C 写失败
解决：检查 SDA/SCL 连接
================
```

---

## 📞 常见问题

### Q: 显示 101 怎么办？
**A**: I2C 写失败。检查：
1. SDA/SCL 接线是否正确
2. 上拉电阻是否焊接
3. I2C 地址是否正确 (0x21/0x22)

### Q: 显示 201 怎么办？
**A**: 芯片初始化失败。检查：
1. 电源电压 (3.3V)
2. 复位电路
3. 配置参数

### Q: 显示 301/302 怎么办？
**A**: 频率超出范围。确保设置在 87.5-108 MHz 之间。

### Q: 如何添加自定义错误码？
**A**: 在 `lcd_error_codes.h` 中添加：
```c
#define ERR_MY_CUSTOM  950  /* 在保留范围内 */
```
然后在 `lcd_error_display.c` 的错误表中添加对应信息。

---

## 📝 版本历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.0 | 2026-03-17 | 初始版本，基础错误码系统 |

---

*基于 Silicon Labs AN332 文档 · 3 位断码屏错误码系统*
