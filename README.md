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
├── si47xx_fm_transmitter.h   # 头文件
├── si47xx_fm_transmitter.c   # 实现
├── example_fm_transmitter.c  # 示例程序
├── Makefile                  # 构建文件
└── README.md                 # 说明文档
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
// 设置频率 (kHz)
si47xx_tx_set_frequency(107900);  // 107.90 MHz

// 设置功率 (dBuV, 77-115)
si47xx_tx_set_power(115);

// 开启发射
si47xx_tx_enable();
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
2. 实现硬件接口函数
3. 调用 API 进行操作

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

### LCD 错误码测试

提供了专门的硬件测试程序，支持 3 位断码屏显示错误码：

- **文件**: `lcd_test_si47xx.c`
- **功能**: 通过 3 位 LCD 屏显示错误码，快速定位硬件问题
- **错误码范围**: 
  - 1xx: I2C 通信错误
  - 2xx: 芯片初始化错误
  - 3xx: 频率设置错误
  - 4xx: 功率设置错误
  - 5xx: RDS 功能错误
  - 6xx: 音频控制错误
  - 7xx: 属性设置错误
  - 8xx: 配置错误
  - 9xx: 系统错误

### 使用方法

1. 实现 LCD 接口函数：
   ```c
   extern void LCD_dispnum(char number);  // 显示单个数字 '0'-'9'
   ```

2. 编译测试程序：
   ```bash
   gcc -I. lcd_test_si47xx.c si47xx_fm_transmitter.c -o lcd_test
   ```

3. 运行测试：
   ```bash
   ./lcd_test
   ```

详细错误码含义请参考 `LCD_HARDWARE_TEST.md`。