/**
 * @file lcd_error_codes.h
 * @brief LCD 错误码定义
 * 
 * 3 位断码屏错误码系统
 * 显示范围：000-999
 */

#ifndef __LCD_ERROR_CODES_H__
#define __LCD_ERROR_CODES_H__

/*============================================================================
 * 错误码分类
 *============================================================================*/

/* 成功状态 */
#define ERR_OK              0       /* 正常运行，无错误 */

/* I2C 通信错误 (1xx) */
#define ERR_I2C_WRITE       101     /* I2C 写失败 */
#define ERR_I2C_READ        102     /* I2C 读失败 */
#define ERR_I2C_TIMEOUT     103     /* I2C 超时 */
#define ERR_I2C_NACK        104     /* I2C 无应答 */

/* 芯片相关错误 (2xx) */
#define ERR_CHIP_NOT_FOUND  200     /* 芯片未检测到 */
#define ERR_CHIP_INIT       201     /* 芯片初始化失败 */
#define ERR_CHIP_VERSION    202     /* 芯片版本不匹配 */
#define ERR_CHIP_RESET      203     /* 芯片复位失败 */

/* 频率相关错误 (3xx) */
#define ERR_FREQ_LOW        301     /* 频率过低 */
#define ERR_FREQ_HIGH       302     /* 频率过高 */
#define ERR_FREQ_LOCK       303     /* 频率锁定失败 */
#define ERR_FREQ_TUNE       304     /* 频率调谐失败 */

/* 功率相关错误 (4xx) */
#define ERR_POWER_LOW       401     /* 功率过低 */
#define ERR_POWER_HIGH      402     /* 功率过高 */
#define ERR_POWER_ADJ       403     /* 功率调整失败 */

/* 音频相关错误 (5xx) */
#define ERR_AUDIO_MUTE      501     /* 音频静音 */
#define ERR_AUDIO_VOL       502     /* 音量设置失败 */
#define ERR_AUDIO_IN        503     /* 音频输入异常 */

/* RDS 相关错误 (6xx) */
#define ERR_RDS_ENABLE      601     /* RDS 使能失败 */
#define ERR_RDS_DATA        602     /* RDS 数据错误 */
#define ERR_RDS_SYNC        603     /* RDS 同步失败 */

/* 系统错误 (7xx) */
#define ERR_SYS_MEMORY      701     /* 内存不足 */
#define ERR_SYS_STACK       702     /* 栈溢出 */
#define ERR_SYS_WATCHDOG    703     /* 看门狗复位 */
#define ERR_SYS_CLOCK       704     /* 系统时钟异常 */

/* 用户错误 (8xx) */
#define ERR_USER_PARAM      801     /* 参数错误 */
#define ERR_USER_RANGE      802     /* 超出范围 */
#define ERR_USER_STATE      803     /* 状态错误 */

/* 保留错误码 */
#define ERR_RESERVED        999     /* 保留/未定义错误 */

/*============================================================================
 * 错误码结构
 *============================================================================*/

typedef struct {
    uint16_t code;          /* 错误码 */
    const char *name;       /* 错误名称 */
    const char *desc;       /* 错误描述 */
    const char *solution;   /* 解决方法 */
} lcd_error_info_t;

/*============================================================================
 * 函数声明
 *============================================================================*/

/**
 * @brief 显示错误码
 * @param code 错误码 (0-999)
 */
void lcd_show_error(uint16_t code);

/**
 * @brief 获取错误信息
 * @param code 错误码
 * @return 错误信息结构
 */
const lcd_error_info_t* lcd_get_error_info(uint16_t code);

/**
 * @brief 清除错误显示
 */
void lcd_clear_error(void);

/**
 * @brief 闪烁错误码 (用于引起注意)
 * @param code 错误码
 * @param times 闪烁次数
 */
void lcd_blink_error(uint16_t code, uint8_t times);

#endif /* __LCD_ERROR_CODES_H__ */
