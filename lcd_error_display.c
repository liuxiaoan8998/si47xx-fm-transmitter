/**
 * @file lcd_error_display.c
 * @brief LCD 错误码显示实现
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "lcd_error_codes.h"

/*============================================================================
 * 外部函数声明 (需要用户实现)
 *============================================================================*/

/**
 * @brief 显示单个数字 (0-9)
 * @param number 数字字符 ('0'-'9')
 */
extern void LCD_dispnum(char number);

/**
 * @brief 清除显示
 */
extern void LCD_clear(void);

/**
 * @brief 延时函数
 * @param ms 毫秒数
 */
extern void delay_ms(uint32_t ms);

/*============================================================================
 * 错误信息表
 *============================================================================*/

static const lcd_error_info_t g_error_table[] = {
    /* 成功 */
    { ERR_OK,           "OK",       "系统正常",           "无需处理" },
    
    /* I2C 错误 */
    { ERR_I2C_WRITE,    "I2C-W",    "I2C 写失败",         "检查 SDA/SCL 连接" },
    { ERR_I2C_READ,     "I2C-R",    "I2C 读失败",         "检查上拉电阻" },
    { ERR_I2C_TIMEOUT,  "I2C-T",    "I2C 超时",           "检查时钟频率" },
    { ERR_I2C_NACK,     "I2C-N",    "I2C 无应答",         "检查设备地址" },
    
    /* 芯片错误 */
    { ERR_CHIP_NOT_FOUND,"CHIP-0",  "芯片未检测到",       "检查电源和复位" },
    { ERR_CHIP_INIT,    "CHIP-I",   "初始化失败",         "检查配置参数" },
    { ERR_CHIP_VERSION, "CHIP-V",   "版本不匹配",         "更新固件" },
    { ERR_CHIP_RESET,   "CHIP-R",   "复位失败",           "检查复位电路" },
    
    /* 频率错误 */
    { ERR_FREQ_LOW,     "F-LOW",    "频率过低",           "调整到 87.5MHz 以上" },
    { ERR_FREQ_HIGH,    "F-HIGH",   "频率过高",           "调整到 108MHz 以下" },
    { ERR_FREQ_LOCK,    "F-LOCK",   "频率锁定失败",       "检查天线" },
    { ERR_FREQ_TUNE,    "F-TUNE",   "调谐失败",           "重试或重启" },
    
    /* 功率错误 */
    { ERR_POWER_LOW,    "P-LOW",    "功率过低",           "检查功率设置" },
    { ERR_POWER_HIGH,   "P-HIGH",   "功率过高",           "降低功率" },
    { ERR_POWER_ADJ,    "P-ADJ",    "功率调整失败",       "检查 PA 电路" },
    
    /* 音频错误 */
    { ERR_AUDIO_MUTE,   "A-MUTE",   "音频静音",           "取消静音" },
    { ERR_AUDIO_VOL,    "A-VOL",    "音量设置失败",       "检查音频通路" },
    { ERR_AUDIO_IN,     "A-IN",     "音频输入异常",       "检查输入信号" },
    
    /* RDS 错误 */
    { ERR_RDS_ENABLE,   "R-ENA",    "RDS 使能失败",       "检查芯片支持" },
    { ERR_RDS_DATA,     "R-DAT",    "RDS 数据错误",       "检查数据格式" },
    { ERR_RDS_SYNC,     "R-SYN",    "RDS 同步失败",       "等待同步" },
    
    /* 系统错误 */
    { ERR_SYS_MEMORY,   "S-MEM",    "内存不足",           "释放内存" },
    { ERR_SYS_STACK,    "S-STK",    "栈溢出",             "优化代码" },
    { ERR_SYS_WATCHDOG, "S-WDG",    "看门狗复位",         "检查程序卡死" },
    { ERR_SYS_CLOCK,    "S-CLK",    "时钟异常",           "检查晶振" },
    
    /* 用户错误 */
    { ERR_USER_PARAM,   "U-PAR",    "参数错误",           "检查输入参数" },
    { ERR_USER_RANGE,   "U-RNG",    "超出范围",           "检查参数范围" },
    { ERR_USER_STATE,   "U-STA",    "状态错误",           "检查调用顺序" },
    
    /* 保留 */
    { ERR_RESERVED,     "UKN",      "未定义错误",         "查阅手册" },
};

#define ERROR_TABLE_SIZE (sizeof(g_error_table) / sizeof(g_error_table[0]))

/*============================================================================
 * 私有变量
 *============================================================================*/

static uint16_t g_current_error = ERR_OK;
static bool g_error_active = false;

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 将数字分解为百十个位
 */
static void split_number(uint16_t num, uint8_t *hundreds, uint8_t *tens, uint8_t *ones)
{
    *hundreds = (num / 100) % 10;
    *tens = (num / 10) % 10;
    *ones = num % 10;
}

/**
 * @brief 显示 3 位数字
 */
static void display_3digits(uint16_t num)
{
    uint8_t h, t, o;
    split_number(num, &h, &t, &o);
    
    /* 显示百位 */
    LCD_dispnum('0' + h);
    delay_ms(5);
    
    /* 显示十位 */
    LCD_dispnum('0' + t);
    delay_ms(5);
    
    /* 显示个位 */
    LCD_dispnum('0' + o);
    delay_ms(5);
}

/*============================================================================
 * 公开函数实现
 *============================================================================*/

void lcd_show_error(uint16_t code)
{
    g_current_error = code;
    g_error_active = true;
    
    /* 清除显示 */
    LCD_clear();
    
    /* 显示错误码 */
    display_3digits(code);
}

void lcd_clear_error(void)
{
    g_current_error = ERR_OK;
    g_error_active = false;
    
    LCD_clear();
}

void lcd_blink_error(uint16_t code, uint8_t times)
{
    for (uint8_t i = 0; i < times; i++) {
        lcd_show_error(code);
        delay_ms(500);
        lcd_clear_error();
        delay_ms(300);
    }
    
    /* 最后保持显示 */
    lcd_show_error(code);
}

const lcd_error_info_t* lcd_get_error_info(uint16_t code)
{
    /* 查找错误码 */
    for (size_t i = 0; i < ERROR_TABLE_SIZE; i++) {
        if (g_error_table[i].code == code) {
            return &g_error_table[i];
        }
    }
    
    /* 未找到，返回未知错误 */
    return &g_error_table[ERROR_TABLE_SIZE - 1];
}

/*============================================================================
 * 调试辅助函数
 *============================================================================*/

/**
 * @brief 打印错误信息到串口 (调试用)
 */
void lcd_print_error_debug(uint16_t code)
{
    const lcd_error_info_t *info = lcd_get_error_info(code);
    
    printf("\r\n=== 错误信息 ===\r\n");
    printf("代码：%d\r\n", info->code);
    printf("名称：%s\r\n", info->name);
    printf("描述：%s\r\n", info->desc);
    printf("解决：%s\r\n", info->solution);
    printf("================\r\n");
}

/**
 * @brief 测试所有错误码显示
 */
void lcd_test_all_errors(void)
{
    printf("开始错误码显示测试...\r\n");
    
    for (size_t i = 0; i < ERROR_TABLE_SIZE; i++) {
        printf("显示错误 %d: %s\r\n", 
               g_error_table[i].code, 
               g_error_table[i].name);
        
        lcd_blink_error(g_error_table[i].code, 2);
        delay_ms(1000);
    }
    
    printf("测试完成\r\n");
}
