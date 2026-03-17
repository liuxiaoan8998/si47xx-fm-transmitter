/**
 * @file lcd_test_si47xx.c
 * @brief Si47xx FM Transmitter 硬件测试程序 (配合3位断码屏)
 * 
 * 错误码定义:
 * 1xx - I2C 通信错误
 * 2xx - 芯片初始化错误  
 * 3xx - 频率设置错误
 * 4xx - 功率设置错误
 * 5xx - RDS 功能错误
 * 6xx - 音频控制错误
 * 7xx - 属性设置错误
 * 8xx - 配置错误
 * 9xx - 系统错误
 */

#include "si47xx_fm_transmitter.h"
#include <stdio.h>
#include <string.h>

/*============================================================================
 * 常量定义
 *============================================================================*/

#ifndef SI47XX_I2C_ADDR_TX
#define SI47XX_I2C_ADDR_TX  0x21    // Si47xx 发射器 I2C 地址
#endif

#ifndef SI47XX_I2C_ADDR_RX
#define SI47XX_I2C_ADDR_RX  0x22    // Si47xx 接收器 I2C 地址
#endif

/*============================================================================
 * 外部函数声明 (硬件接口)
 *============================================================================*/

// 假设已有函数
extern void LCD_dispnum(char number);  // 显示单个数字 '0'-'9'

/*============================================================================
 * 错误码定义
 *============================================================================*/

// I2C 通信错误 (1xx)
#define LCD_TEST_I2C_WRITE_ERROR        101
#define LCD_TEST_I2C_READ_ERROR         102
#define LCD_TEST_I2C_NO_RESPONSE        103
#define LCD_TEST_I2C_TIMEOUT            104

// 芯片初始化错误 (2xx)
#define LCD_TEST_CHIP_INIT_FAILED       201
#define LCD_TEST_CHIP_POWERUP_FAILED    202
#define LCD_TEST_CHIP_GET_REV_FAILED    203
#define LCD_TEST_CHIP_NOT_DETECTED      204

// 频率设置错误 (3xx)
#define LCD_TEST_FREQ_OUT_OF_RANGE      301
#define LCD_TEST_FREQ_SET_FAILED        302
#define LCD_TEST_FREQ_INVALID           303
#define LCD_TEST_FREQ_TUNING_FAILED     304

// 功率设置错误 (4xx)
#define LCD_TEST_POWER_OUT_OF_RANGE     401
#define LCD_TEST_POWER_SET_FAILED       402
#define LCD_TEST_POWER_INVALID          403

// RDS 功能错误 (5xx)
#define LCD_TEST_RDS_INIT_FAILED        501
#define LCD_TEST_RDS_SET_FAILED         502
#define LCD_TEST_RDS_UNSUPPORTED        503

// 音频控制错误 (6xx)
#define LCD_TEST_AUDIO_VOL_SET_FAILED   601
#define LCD_TEST_AUDIO_MUTE_FAILED      602

// 属性设置错误 (7xx)
#define LCD_TEST_PROP_SET_FAILED        701
#define LCD_TEST_PROP_GET_FAILED        702

// 配置错误 (8xx)
#define LCD_TEST_CONFIG_INVALID         801
#define LCD_TEST_CONFIG_MISSING         802

// 系统错误 (9xx)
#define LCD_TEST_SYS_MEM_ERROR          901
#define LCD_TEST_SYS_TIMEOUT            902

/*============================================================================
 * LCD 3位数字显示函数
 *============================================================================*/

/**
 * @brief 在3位LCD屏上显示3位数字
 * @param code 3位错误码 (如 101, 201 等)
 */
void LCD_display_code(int code) {
    char hundreds, tens, units;
    
    // 限制在 0-999 范围内
    if (code < 0) code = 0;
    if (code > 999) code = 999;
    
    // 分解为百位、十位、个位
    hundreds = (code / 100) % 10 + '0';
    tens = (code / 10) % 10 + '0';
    units = code % 10 + '0';
    
    // 显示百位
    LCD_dispnum(hundreds);
    si47xx_delay_ms(50);  // 短暂延时确保显示稳定
    
    // 显示十位
    LCD_dispnum(tens);
    si47xx_delay_ms(50);
    
    // 显示个位
    LCD_dispnum(units);
    si47xx_delay_ms(50);
}

/**
 * @brief 显示两位数字（用于简单状态）
 * @param code 两位状态码 (如 10, 20 等)
 */
void LCD_display_code_simple(int code) {
    char tens, units;
    
    if (code < 0) code = 0;
    if (code > 99) code = 99;
    
    tens = (code / 10) % 10 + '0';
    units = code % 10 + '0';
    
    // 显示十位
    LCD_dispnum(tens);
    si47xx_delay_ms(100);
    
    // 显示个位
    LCD_dispnum(units);
    si47xx_delay_ms(100);
    
    // 第三位显示0
    LCD_dispnum('0');
    si47xx_delay_ms(100);
}

/**
 * @brief 显示测试阶段状态
 * @param stage 测试阶段编号 (1-9)
 */
void LCD_display_stage(int stage) {
    LCD_dispnum('0');  // 百位
    LCD_dispnum('0');  // 十位
    LCD_dispnum(stage + '0');  // 个位
    si47xx_delay_ms(200);
}

/*============================================================================
 * 测试函数
 *============================================================================*/

/**
 * @brief 测试 I2C 通信
 * @return 0=成功, 非0=错误码
 */
int test_i2c_communication(void) {
    uint8_t cmd[2] = {0x10, 0x00};  // GET_REV 命令
    uint8_t resp[16];
    
    LCD_display_stage(1);  // 显示测试阶段 1
    
    // 尝试发送命令并接收响应
    if (si47xx_hw_write(SI47XX_I2C_ADDR_TX, cmd, 1) != 0) {
        LCD_display_code(LCD_TEST_I2C_WRITE_ERROR);  // 显示 101
        return LCD_TEST_I2C_WRITE_ERROR;
    }
    
    si47xx_delay_ms(10);
    
    if (si47xx_hw_read(SI47XX_I2C_ADDR_TX, resp, 1) != 0) {
        LCD_display_code(LCD_TEST_I2C_READ_ERROR);  // 显示 102
        return LCD_TEST_I2C_READ_ERROR;
    }
    
    // 检查CTS位
    if ((resp[0] & 0x80) == 0) {
        LCD_display_code(LCD_TEST_I2C_NO_RESPONSE);  // 显示 103
        return LCD_TEST_I2C_NO_RESPONSE;
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试芯片初始化
 * @return 0=成功, 非0=错误码
 */
int test_chip_initialization(void) {
    si47xx_config_t config = {0};
    si47xx_status_t status;
    
    LCD_display_stage(2);  // 显示测试阶段 2
    
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    config.tx_power = 100;
    
    status = si47xx_init(&config);
    if (status != SI47XX_OK) {
        if (status == SI47XX_ERR_BUS_ERROR) {
            LCD_display_code(LCD_TEST_CHIP_INIT_FAILED);  // 显示 201
            return LCD_TEST_CHIP_INIT_FAILED;
        } else if (status == SI47XX_ERR_DEVICE_NOT_READY) {
            LCD_display_code(LCD_TEST_CHIP_NOT_DETECTED);  // 显示 204
            return LCD_TEST_CHIP_NOT_DETECTED;
        } else {
            LCD_display_code(LCD_TEST_CHIP_POWERUP_FAILED);  // 显示 202
            return LCD_TEST_CHIP_POWERUP_FAILED;
        }
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试频率设置
 * @return 0=成功, 非0=错误码
 */
int test_frequency_setting(void) {
    si47xx_status_t status;
    
    LCD_display_stage(3);  // 显示测试阶段 3
    
    // 测试正常频率
    status = si47xx_tx_set_frequency(100000U);  // 100.0 MHz
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_FREQ_SET_FAILED);  // 显示 302
        return LCD_TEST_FREQ_SET_FAILED;
    }
    
    // 测试边界频率
    status = si47xx_tx_set_frequency(87500U);  // 87.5 MHz
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_FREQ_TUNING_FAILED);  // 显示 304
        return LCD_TEST_FREQ_TUNING_FAILED;
    }
    
    // 测试高频
    status = si47xx_tx_set_frequency(108000U);  // 108.0 MHz
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_FREQ_TUNING_FAILED);  // 显示 304
        return LCD_TEST_FREQ_TUNING_FAILED;
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试功率设置
 * @return 0=成功, 非0=错误码
 */
int test_power_setting(void) {
    si47xx_status_t status;
    
    LCD_display_stage(4);  // 显示测试阶段 4
    
    // 测试正常功率
    status = si47xx_tx_set_power(100);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_POWER_SET_FAILED);  // 显示 402
        return LCD_TEST_POWER_SET_FAILED;
    }
    
    // 测试最小功率
    status = si47xx_tx_set_power(77);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_POWER_SET_FAILED);  // 显示 402
        return LCD_TEST_POWER_SET_FAILED;
    }
    
    // 测试最大功率
    status = si47xx_tx_set_power(115);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_POWER_SET_FAILED);  // 显示 402
        return LCD_TEST_POWER_SET_FAILED;
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试RDS功能
 * @return 0=成功, 非0=错误码
 */
int test_rds_functionality(void) {
    si47xx_status_t status;
    
    LCD_display_stage(5);  // 显示测试阶段 5
    
    // 尝试启用RDS (对于不支持RDS的芯片会失败)
    status = si47xx_rds_enable();
    if (status != SI47XX_OK) {
        // 不是致命错误，某些芯片不支持RDS
        // 显示警告但继续测试
        LCD_display_code_simple(50);  // 显示 050 - RDS 不支持
        si47xx_delay_ms(1000);  // 显示1秒
    }
    
    // 如果支持RDS，测试设置PI码
    if (status == SI47XX_OK) {
        status = si47xx_rds_set_pi(0x1234, "TEST");
        if (status != SI47XX_OK) {
            LCD_display_code(LCD_TEST_RDS_SET_FAILED);  // 显示 502
            return LCD_TEST_RDS_SET_FAILED;
        }
        
        status = si47xx_rds_set_text("Si47xx Test");
        if (status != SI47XX_OK) {
            LCD_display_code(LCD_TEST_RDS_SET_FAILED);  // 显示 502
            return LCD_TEST_RDS_SET_FAILED;
        }
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试音频控制
 * @return 0=成功, 非0=错误码
 */
int test_audio_control(void) {
    si47xx_status_t status;
    
    LCD_display_stage(6);  // 显示测试阶段 6
    
    status = si47xx_audio_set_volume(50);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_AUDIO_VOL_SET_FAILED);  // 显示 601
        return LCD_TEST_AUDIO_VOL_SET_FAILED;
    }
    
    status = si47xx_audio_set_mute(false);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_AUDIO_MUTE_FAILED);  // 显示 602
        return LCD_TEST_AUDIO_MUTE_FAILED;
    }
    
    return 0;  // 成功
}

/**
 * @brief 测试属性设置
 * @return 0=成功, 非0=错误码
 */
int test_property_setting(void) {
    si47xx_status_t status;
    uint16_t value;
    
    LCD_display_stage(7);  // 显示测试阶段 7
    
    // 设置属性
    status = si47xx_set_property(0x3702, 100);  // TX_POWER_LEVEL
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_PROP_SET_FAILED);  // 显示 701
        return LCD_TEST_PROP_SET_FAILED;
    }
    
    // 读取属性
    status = si47xx_get_property(0x3702, &value);
    if (status != SI47XX_OK) {
        LCD_display_code(LCD_TEST_PROP_GET_FAILED);  // 显示 702
        return LCD_TEST_PROP_GET_FAILED;
    }
    
    return 0;  // 成功
}

/*============================================================================
 * 主测试函数
 *============================================================================*/

/**
 * @brief 完整的Si47xx硬件测试
 * @return 0=全部成功, 非0=第一个失败的错误码
 */
int si47xx_hardware_test(void) {
    int result;
    
    printf("开始 Si47xx 硬件测试...\n");
    
    // 阶段1: I2C通信测试
    result = test_i2c_communication();
    if (result != 0) {
        printf("I2C通信测试失败: %d\n", result);
        return result;
    }
    printf("I2C通信测试通过\n");
    
    // 阶段2: 芯片初始化测试
    result = test_chip_initialization();
    if (result != 0) {
        printf("芯片初始化测试失败: %d\n", result);
        return result;
    }
    printf("芯片初始化测试通过\n");
    
    // 阶段3: 频率设置测试
    result = test_frequency_setting();
    if (result != 0) {
        printf("频率设置测试失败: %d\n", result);
        return result;
    }
    printf("频率设置测试通过\n");
    
    // 阶段4: 功率设置测试
    result = test_power_setting();
    if (result != 0) {
        printf("功率设置测试失败: %d\n", result);
        return result;
    }
    printf("功率设置测试通过\n");
    
    // 阶段5: RDS功能测试
    result = test_rds_functionality();
    if (result != 0) {
        printf("RDS功能测试失败: %d\n", result);
        return result;
    }
    printf("RDS功能测试通过\n");
    
    // 阶段6: 音频控制测试
    result = test_audio_control();
    if (result != 0) {
        printf("音频控制测试失败: %d\n", result);
        return result;
    }
    printf("音频控制测试通过\n");
    
    // 阶段7: 属性设置测试
    result = test_property_setting();
    if (result != 0) {
        printf("属性设置测试失败: %d\n", result);
        return result;
    }
    printf("属性设置测试通过\n");
    
    // 所有测试通过
    LCD_display_code_simple(0);  // 显示 000 - 测试通过
    printf("所有测试通过！设备正常工作。\n");
    
    return 0;
}

/*============================================================================
 * 简化版快速测试
 *============================================================================*/

/**
 * @brief 快速测试 - 只测试核心功能
 * @return 0=成功, 非0=错误码
 */
int si47xx_quick_test(void) {
    int result;
    
    printf("快速测试开始...\n");
    
    // 快速初始化测试
    si47xx_config_t config = {0};
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    
    result = si47xx_init(&config);
    if (result != SI47XX_OK) {
        LCD_display_code(LCD_TEST_CHIP_INIT_FAILED);
        return LCD_TEST_CHIP_INIT_FAILED;
    }
    
    // 快速频率设置测试
    result = si47xx_tx_set_frequency(100000U);
    if (result != SI47XX_OK) {
        LCD_display_code(LCD_TEST_FREQ_SET_FAILED);
        return LCD_TEST_FREQ_SET_FAILED;
    }
    
    // 快速功率设置测试
    result = si47xx_tx_set_power(100);
    if (result != SI47XX_OK) {
        LCD_display_code(LCD_TEST_POWER_SET_FAILED);
        return LCD_TEST_POWER_SET_FAILED;
    }
    
    // 测试通过
    LCD_display_code_simple(0);  // 显示 000
    printf("快速测试通过！\n");
    
    return 0;
}