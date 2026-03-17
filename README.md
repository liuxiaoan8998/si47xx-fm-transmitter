# Si47xx FM Transmitter Driver

基于 Silicon Labs AN332 的 Si47xx FM Transmitter 驱动库

## 支持芯片

| 芯片 | 功能 |
|------|------|
| Si4710 | FM 发射器 |
| Si4711 | FM 发射器 + RDS |
| Si4712 | FM 发射器 + RPS |
| Si4713 | FM 发射器 + RDS + RPS |
| Si4720 | FM 收发器 |
| Si4721 | FM 收发器 + RDS |

## 文件结构

```
si47xx-fm-transmitter/
├── si47xx_fm_transmitter.h   # 驱动头文件
├── si47xx_fm_transmitter.c   # 驱动实现
├── example_fm_transmitter.c  # 示例程序
├── test_si47xx.c             # 单元测试
├── lcd_simple_test.c         # 简化 LCD 测试 (2 位屏)
├── lcd_test_si47xx.c         # 完整 LCD 测试 (3 位屏)
├── lcd_error_display.c       # LCD 错误显示驱动
├── Makefile                  # 构建文件
├── README.md                 # 本说明文档
├── LCD_ERROR_CODES.md        # 错误码速查表
└── LCD_HARDWARE_TEST.md      # 硬件测试指南
```

## 快速开始

### 1. 实现硬件接口

在 `example_fm_transmitter.c` 中实现以下函数:

```c
extern int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len);
extern int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len);
extern void si47xx_delay_ms(uint32_t ms);
```

### 2. 编译

```bash
make
```

### 3. 运行示例

```bash
./example_fm_transmitter
```

## API 速查

### 初始化

```c
si47xx_config_t config = {0};
config.band = BAND_US_EU;
config.freq_min = 87500;  // 87.5 MHz
config.freq_max = 108000; // 108 MHz
config.tx_power = 115;

si47xx_init(&config);
```

### 设置发射

```c
// 方式 1: 分步设置
si47xx_tx_set_frequency(107900);  // 设置频率 (kHz)
si47xx_tx_set_power(115);         // 设置功率 (dBuV, 77-115)

// 方式 2: 使用 tx_enable (自动设置默认值)
si47xx_tx_enable();  // 默认 87.5 MHz, 115 dBuV
```

### 音频控制

```c
// 设置音量 (0-100)
si47xx_audio_set_volume(80);

// 静音
si47xx_audio_set_mute(true);
```

### RDS (需要Si4711/13/21)

```c
// 启用RDS
si47xx_rds_enable();

// 设置节目信息
si47xx_rds_set_pi(0x1234, "MY_FM");

// 设置广播文本
si47xx_rds_set_text("Hello FM World!");
```

## 嵌入式项目移植

1. 复制 `si47xx_fm_transmitter.h` 和 `si47xx_fm_transmitter.c` 到你的项目
2. 实现硬件接口函数：
   ```c
   int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len);
   int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len);
   void si47xx_delay_ms(uint32_t ms);
   ```
3. 调用 API 进行操作

### 典型使用流程

```c
// 1. 初始化
si47xx_config_t config = {0};
config.band = BAND_US_EU;
config.freq_min = 87500U;
config.freq_max = 108000U;
si47xx_init(&config);

// 2. 设置频率和功率
si47xx_tx_set_frequency(107900U);  // 107.9 MHz
si47xx_tx_set_power(100);          // 100 dBuV

// 3. 可选：配置音频
si47xx_audio_set_volume(80);

// 4. 可选：RDS (仅 Si4711/13/21)
si47xx_rds_enable();
si47xx_rds_set_pi(0x1234, "MY_FM");
```

## 硬件连接

```
Si47xx (TX Mode)
----------------
VIO   -> 3.3V
GND   -> GND
SDA   -> I2C SDA
SCL   -> I2C SCL
RST   -> GPIO (复位)
GPO1  -> 天线 (RF输出)
```

## I2C 地址

| 模式 | 地址 |
|------|------|
| 发射器 (TX) | 0x21 |
| 接收器 (RX) | 0x22 |

## 许可证

基于 Silicon Labs AN332 文档

## 硬件测试

### 1. 简化版测试（2 位 LCD 屏）

推荐使用简化版测试程序：

- **文件**: `lcd_simple_test.c`
- **编译**: `make lcd_simple`
- **运行**: `./lcd_simple_test`
- **错误码范围**: 10-99

| 错误码 | 含义 |
|--------|------|
| 10 | I2C 写失败 |
| 11 | I2C 读失败 |
| 12 | I2C 超时 |
| 20 | 芯片初始化失败 |
| 21 | 芯片检测失败 |
| 30 | 频率范围错误 |
| 31 | 频率设置失败 |
| 40 | 功率范围错误 |
| 41 | 功率设置失败 |
| 99 | 测试通过 |

### 2. 完整版测试（3 位 LCD 屏）

如果需要更详细的错误诊断：

- **文件**: `lcd_test_si47xx.c`
- **编译**: `make lcd_test`
- **运行**: `./test_with_lcd`
- **错误码范围**: 101-999（详细分类）

详细错误码含义参考 `LCD_HARDWARE_TEST.md` 和 `LCD_ERROR_CODES.md`。

### LCD 接口

两个测试程序都需要实现：
```c
extern void LCD_dispnum(char number);  // 显示单个数字 '0'-'9'
extern void si47xx_delay_ms(uint32_t ms);  // 毫秒延时
```