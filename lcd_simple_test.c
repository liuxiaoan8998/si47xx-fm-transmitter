/**
 * @file lcd_simple_test.c
 * @brief Si47xx FM Transmitter 简化 LCD 测试程序
 * 
 * 错误码范围：10-99（2 位数字）
 */

#include "si47xx_fm_transmitter.h"
#include <stdio.h>

/*============================================================================
 * 外部函数声明
 *============================================================================*/

extern void LCD_dispnum(char number);  // 显示单个数字 '0'-'9'

/*============================================================================
 * 错误码定义 (10-99)
 *============================================================================*/

// I2C 通信错误 (10-19)
#define ERR_I2C_WRITE       10
#define ERR_I2C_READ        11
#define ERR_I2C_TIMEOUT     12

// 芯片初始化错误 (20-29)
#define ERR_CHIP_INIT       20
#define ERR_CHIP_DETECT     21

// 频率设置错误 (30-39)
#define ERR_FREQ_RANGE      30
#define ERR_FREQ_SET        31

// 功率设置错误 (40-49)
#define ERR_POWER_RANGE     40
#define ERR_POWER_SET       41

// 测试通过
#define TEST_OK             99

/*============================================================================
 * 测试函数
 *============================================================================*/

/**
 * @brief I2C 通信测试
 */
int test_i2c(void) {
    uint8_t cmd = 0x10;  // GET_REV
    uint8_t resp;
    
    if (si47xx_hw_write(0x21, &cmd, 1) != 0) {
        LCD_dispnum(ERR_I2C_WRITE / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_I2C_WRITE % 10); si47xx_delay_ms(100);
        return ERR_I2C_WRITE;
    }
    
    si47xx_delay_ms(10);
    
    if (si47xx_hw_read(0x21, &resp, 1) != 0) {
        LCD_dispnum(ERR_I2C_READ / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_I2C_READ % 10); si47xx_delay_ms(100);
        return ERR_I2C_READ;
    }
    
    if ((resp & 0x80) == 0) {
        LCD_dispnum(ERR_I2C_TIMEOUT / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_I2C_TIMEOUT % 10); si47xx_delay_ms(100);
        return ERR_I2C_TIMEOUT;
    }
    
    return 0;
}

/**
 * @brief 芯片初始化测试
 */
int test_init(void) {
    si47xx_config_t config = {0};
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    
    if (si47xx_init(&config) != SI47XX_OK) {
        LCD_dispnum(ERR_CHIP_INIT / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_CHIP_INIT % 10); si47xx_delay_ms(100);
        return ERR_CHIP_INIT;
    }
    
    return 0;
}

/**
 * @brief 频率设置测试
 */
int test_freq(void) {
    if (si47xx_tx_set_frequency(100000U) != SI47XX_OK) {
        LCD_dispnum(ERR_FREQ_SET / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_FREQ_SET % 10); si47xx_delay_ms(100);
        return ERR_FREQ_SET;
    }
    return 0;
}

/**
 * @brief 功率设置测试
 */
int test_power(void) {
    if (si47xx_tx_set_power(100) != SI47XX_OK) {
        LCD_dispnum(ERR_POWER_SET / 10); si47xx_delay_ms(100);
        LCD_dispnum(ERR_POWER_SET % 10); si47xx_delay_ms(100);
        return ERR_POWER_SET;
    }
    return 0;
}

/*============================================================================
 * 主测试函数
 *============================================================================*/

/**
 * @brief 完整测试流程
 * @return 0=成功，非 0=错误码
 */
int si47xx_test(void) {
    int result;
    
    printf("Si47xx 测试开始...\n");
    
    result = test_i2c();
    if (result != 0) return result;
    printf("I2C OK\n");
    
    result = test_init();
    if (result != 0) return result;
    printf("Init OK\n");
    
    result = test_freq();
    if (result != 0) return result;
    printf("Freq OK\n");
    
    result = test_power();
    if (result != 0) return result;
    printf("Power OK\n");
    
    // 全部通过，显示 99
    LCD_dispnum(TEST_OK / 10); si47xx_delay_ms(100);
    LCD_dispnum(TEST_OK % 10); si47xx_delay_ms(100);
    printf("测试通过！\n");
    
    return 0;
}
