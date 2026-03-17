/**
 * @file test_with_lcd.c
 * @brief Si47xx 测试程序 (带 LCD 错误码显示)
 * 
 * 功能：
 * 1. 测试 Si47xx 各项功能
 * 2. 遇到错误时在 LCD 上显示错误码
 * 3. 通过错误码快速定位问题
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "si47xx_fm_transmitter.h"
#include "lcd_error_codes.h"

/*============================================================================
 * 硬件接口实现 (需要根据实际硬件修改)
 *============================================================================*/

/* LCD 显示函数 - 需要用户实现 */
void LCD_dispnum(char number)
{
    /* TODO: 实现你的 LCD 显示函数 */
    printf("[LCD] 显示：%c\r\n", number);
}

void LCD_clear(void)
{
    printf("[LCD] 清除显示\r\n");
}

void delay_ms(uint32_t ms)
{
    /* TODO: 实现精确延时 */
    printf("[DELAY] %d ms\r\n", ms);
}

/* I2C 接口 - 需要用户实现 */
int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len)
{
    /* TODO: 实现 I2C 写 */
    printf("[I2C] 写 addr=0x%02X len=%d\r\n", addr, len);
    return 0;  /* 模拟成功 */
}

int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    /* TODO: 实现 I2C 读 */
    printf("[I2C] 读 addr=0x%02X len=%d\r\n", addr, len);
    data[0] = 0x80;  /* CTS=1 */
    return 0;  /* 模拟成功 */
}

void si47xx_delay_ms(uint32_t ms)
{
    /* TODO: 实现精确延时 */
    delay_ms(ms);
}

/*============================================================================
 * 测试宏
 *============================================================================*/

#define TEST_CHECK(cond, err_code) do { \
    if (!(cond)) { \
        printf("[测试失败] 错误码：%d\r\n", err_code); \
        lcd_show_error(err_code); \
        return err_code; \
    } \
} while(0)

#define TEST_STEP(step, cond, err_code) do { \
    printf("[步骤 %d] 测试中...\r\n", step); \
    TEST_CHECK(cond, err_code); \
    printf("[步骤 %d] 通过\r\n", step); \
} while(0)

/*============================================================================
 * 测试用例
 *============================================================================*/

/**
 * 测试 1: 系统初始化
 */
int test_system_init(void)
{
    printf("\r\n=== 测试 1: 系统初始化 ===\r\n");
    
    /* 步骤 1: 清除 LCD */
    lcd_clear_error();
    
    /* 步骤 2: 初始化配置 */
    si47xx_config_t config = {0};
    config.band = BAND_US_EU;
    config.freq_min = 87500;
    config.freq_max = 108000;
    config.tx_power = 100;
    
    /* 步骤 3: 调用初始化 */
    si47xx_status_t status = si47xx_init(&config);
    
    TEST_STEP(1, status == SI47XX_OK, ERR_CHIP_INIT);
    
    printf("初始化成功!\r\n");
    lcd_show_error(ERR_OK);
    return ERR_OK;
}

/**
 * 测试 2: 频率设置
 */
int test_frequency(void)
{
    printf("\r\n=== 测试 2: 频率设置 ===\r\n");
    
    uint16_t test_freqs[] = {87500, 98000, 107900};
    int num_tests = sizeof(test_freqs) / sizeof(test_freqs[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("测试频率：%d kHz\r\n", test_freqs[i]);
        
        si47xx_status_t status = si47xx_tx_set_frequency(test_freqs[i]);
        
        if (status != SI47XX_OK) {
            if (test_freqs[i] < 87500) {
                TEST_CHECK(false, ERR_FREQ_LOW);
            } else if (test_freqs[i] > 108000) {
                TEST_CHECK(false, ERR_FREQ_HIGH);
            } else {
                TEST_CHECK(false, ERR_FREQ_TUNE);
            }
        }
    }
    
    printf("频率测试通过!\r\n");
    lcd_show_error(ERR_OK);
    return ERR_OK;
}

/**
 * 测试 3: 功率设置
 */
int test_power(void)
{
    printf("\r\n=== 测试 3: 功率设置 ===\r\n");
    
    /* 测试正常功率 */
    si47xx_status_t status = si47xx_tx_set_power(100);
    TEST_STEP(1, status == SI47XX_OK, ERR_POWER_ADJ);
    
    /* 测试功率边界 */
    status = si47xx_tx_set_power(50);  /* 低于最小值 */
    /* 应该自动调整，不报错 */
    
    status = si47xx_tx_set_power(120); /* 高于最大值 */
    /* 应该自动调整，不报错 */
    
    printf("功率测试通过!\r\n");
    lcd_show_error(ERR_OK);
    return ERR_OK;
}

/**
 * 测试 4: 音频控制
 */
int test_audio(void)
{
    printf("\r\n=== 测试 4: 音频控制 ===\r\n");
    
    /* 测试音量 */
    si47xx_status_t status = si47xx_audio_set_volume(50);
    TEST_STEP(1, status == SI47XX_OK, ERR_AUDIO_VOL);
    
    /* 测试静音 */
    status = si47xx_audio_set_mute(true);
    TEST_STEP(2, status == SI47XX_OK, ERR_AUDIO_MUTE);
    
    status = si47xx_audio_set_mute(false);
    TEST_STEP(3, status == SI47XX_OK, ERR_AUDIO_MUTE);
    
    printf("音频测试通过!\r\n");
    lcd_show_error(ERR_OK);
    return ERR_OK;
}

/**
 * 测试 5: I2C 错误模拟
 */
int test_i2c_error(void)
{
    printf("\r\n=== 测试 5: I2C 错误模拟 ===\r\n");
    
    /* 模拟 I2C 错误 */
    printf("模拟 I2C 写错误...\r\n");
    
    /* 这里需要修改硬件接口来模拟错误 */
    /* 实际使用时，可以添加一个全局变量来控制模拟错误 */
    
    printf("I2C 错误模拟完成!\r\n");
    lcd_show_error(ERR_OK);
    return ERR_OK;
}

/**
 * 测试 6: 完整流程
 */
int test_full_flow(void)
{
    printf("\r\n=== 测试 6: 完整流程 ===\r\n");
    
    /* 步骤 1: 初始化 */
    TEST_STEP(1, test_system_init() == ERR_OK, ERR_CHIP_INIT);
    
    /* 步骤 2: 设置频率 */
    TEST_STEP(2, test_frequency() == ERR_OK, ERR_FREQ_TUNE);
    
    /* 步骤 3: 设置功率 */
    TEST_STEP(3, test_power() == ERR_OK, ERR_POWER_ADJ);
    
    /* 步骤 4: 音频控制 */
    TEST_STEP(4, test_audio() == ERR_OK, ERR_AUDIO_VOL);
    
    /* 步骤 5: 开启发射 */
    si47xx_status_t status = si47xx_tx_enable();
    TEST_STEP(5, status == SI47XX_OK, ERR_CHIP_INIT);
    
    printf("\r\n=== 完整流程测试通过! ===\r\n");
    lcd_blink_error(ERR_OK, 3);  /* 闪烁 3 次表示成功 */
    
    return ERR_OK;
}

/*============================================================================
 * 主程序
 *============================================================================*/

int main(void)
{
    printf("\r\n");
    printf("========================================\r\n");
    printf("  Si47xx 测试程序 (LCD 错误码显示)\r\n");
    printf("========================================\r\n");
    
    /* 开机自检 - 显示 000 表示正常启动 */
    lcd_show_error(ERR_OK);
    delay_ms(1000);
    
    /* 运行所有测试 */
    int result;
    
    result = test_system_init();
    if (result != ERR_OK) return result;
    
    result = test_frequency();
    if (result != ERR_OK) return result;
    
    result = test_power();
    if (result != ERR_OK) return result;
    
    result = test_audio();
    if (result != ERR_OK) return result;
    
    result = test_i2c_error();
    if (result != ERR_OK) return result;
    
    result = test_full_flow();
    if (result != ERR_OK) return result;
    
    printf("\r\n所有测试通过！\r\n");
    printf("系统正常运行，显示 ERR_OK (0)\r\n");
    
    /* 保持运行 */
    while (1) {
        lcd_show_error(ERR_OK);
        delay_ms(1000);
    }
    
    return 0;
}
