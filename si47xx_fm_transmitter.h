/**
 * @file si47xx_fm_transmitter.h
 * @brief Silicon Labs Si47xx FM Transmitter Driver
 * @version 1.0
 * 
 * 基于 AN332 Si47xx Programming Guide
 * 支持 Si4710/11/12/13/20/21 FM Transmitter
 * 
 * 使用方法:
 *   1. 实现 si47xx_hw_read()/si47xx_hw_write() 硬件接口
 *   2. 调用 si47xx_init() 初始化
 *   3. 调用 API 函数进行操作
 */

#ifndef __SI47XX_FM_TRANSMITTER_H__
#define __SI47XX_FM_TRANSMITTER_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*============================================================================
 * 配置选项
 *============================================================================*/

#define SI47XX_MAX_DELAY_MS      300
#define SI47XX_CMD_TIMEOUT       100
#define SI47XX_CTS_TIMEOUT        500

/*============================================================================
 * 数据类型定义
 *============================================================================*/

/* 响应状态 */
typedef enum {
    SI47XX_OK = 0,
    SI47XX_ERR_CTS_TIMEOUT,      // CTS 超时
    SI47XX_ERR_INVALID_ARG,      // 无效参数
    SI47XX_ERR_BUS_ERROR,         // 总线错误
    SI47XX_ERR_DEVICE_NOT_READY,  // 设备未就绪
    SI47XX_ERR_INVALID_DEVICE,    // 无效设备
    SI47XX_ERR_TUNE_FAILED,       // 调谐失败
    SI47XX_ERR_SEEK_FAILED,       // 搜索失败
} si47xx_status_t;

/* 芯片型号 */
typedef enum {
    SI47XX_UNKNOWN = 0,
    SI4710,           // FM Transmitter
    SI4711,           // FM Transmitter with RDS
    SI4712,           // FM Transmitter with RPS
    SI4713,           // FM Transmitter with RDS & RPS
    SI4720,           // FM Transceiver
    SI4721,           // FM Transceiver with RDS
} si47xx_device_t;

/* 频段 */
typedef enum {
    BAND_US_EU = 0,   // 87.5 - 108 MHz
    BAND_JAPAN,       // 76 - 108 MHz  
    BAND_WORLD,        // 76 - 108 MHz
    BAND_EAST_JAPAN,   // 76 - 90 MHz
} si47xx_band_t;

/* 预加重 */
typedef enum {
    PREEMPHASIS_USA = 0,   // 75 us
    PREEMPHASIS_EU,        // 50 us
    PREEMPHASIS_NONE,      // 无
} si47xx_preemphasis_t;

/* 发射功率 */
typedef enum {
    TX_POWER_115_DBU = 0,    // 115 dBuV
    TX_POWER_110_DBU,        // 110 dBuV
    TX_POWER_105_DBU,        // 105 dBuV
    TX_POWER_100_DBU,        // 100 dBuV
    TX_POWER_95_DBU,         // 95 dBuV
    TX_POWER_90_DBU,         // 90 dBuV
    TX_POWER_85_DBU,         // 85 dBuV
    TX_POWER_77_DBU,         // 77 dBuV
} si47xx_tx_power_t;

/* RDS 消息类型 */
typedef enum {
    RDS_TYPE_NONE = 0,
    RDS_TYPE_PI,         // 节目识别
    RDS_TYPE_PS,         // 节目服务名称
    RDS_TYPE_RT,         // 广播文本
    RDS_TYPE_CT,         // 实时时钟
    RDS_TYPE_TP,         // 交通节目
    RDS_TYPE_TA,         // 交通公告
    RDS_TYPE_AF,         // 替代频率
} si47xx_rds_type_t;

/*============================================================================
 * 数据结构
 *============================================================================*/

/* 芯片信息 */
typedef struct {
    si47xx_device_t  device;
    uint8_t          part_number;
    uint8_t          firmware_major;
    uint8_t          firmware_minor;
    uint16_t         patch_id;
    uint16_t         patch_major;
    uint16_t         patch_minor;
    char             chip_name[16];
} si47xx_info_t;

/* 发射状态 */
typedef struct {
    uint16_t         frequency;      // kHz, 例如 10790 = 107.90 MHz
    int8_t           rssi;           // dBm
    uint8_t          tx_power;       // dBuV
    bool             tx_enabled;
    bool             rds_enabled;
} si47xx_tx_status_t;

/* RDS 消息 */
typedef struct {
    si47xx_rds_type_t type;
    uint16_t          pi_code;           // 节目识别码
    char              ps_name[9];         // 8字符 + 结束符
    char              radio_text[65];    // 64字符 + 结束符
    bool              tp_flag;            // 交通节目标志
    bool              ta_flag;            // 交通公告标志
    uint8_t          af_count;           // 替代频率数量
    uint16_t          af_list[25];        // 替代频率列表
} si47xx_rds_data_t;

/* 音频配置 */
typedef struct {
    uint8_t           volume;            // 0-100
    bool               mute;
    int8_t             left_gain;         // dB
    int8_t             right_gain;        // dB
    uint8_t            audio_mode;        // 0=Analog, 1=Digital
} si47xx_audio_config_t;

/* 配置结构 */
typedef struct {
    si47xx_band_t          band;           // 频段
    si47xx_preemphasis_t   preemphasis;    // 预加重
    uint32_t               freq_min;       // 最小频率 (kHz)
    uint32_t               freq_max;       // 最大频率 (kHz)
    uint8_t                tx_power;       // 发射功率
    si47xx_audio_config_t  audio;
} si47xx_config_t;

/*============================================================================
 * 硬件抽象层接口 (需要用户实现)
 *============================================================================*/

/**
 * @brief 硬件写函数
 * @param addr 设备地址 (0x21 for TX, 0x22 for RX)
 * @param data 写入数据
 * @param len  数据长度
 * @return 0=成功, 其他=错误码
 */
extern int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len);

/**
 * @brief 硬件读函数
 * @param addr 设备地址 (0x21 for TX, 0x22 for RX)
 * @param data 读取数据缓冲区
 * @param len  读取长度
 * @return 0=成功, 其他=错误码
 */
extern int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len);

/**
 * @brief 毫秒延时函数
 * @param ms 延时毫秒数
 */
extern void si47xx_delay_ms(uint32_t ms);

/*============================================================================
 * 核心 API 函数
 *============================================================================*/

/**
 * @brief 初始化 Si47xx 芯片
 * @param config 配置参数
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_init(const si47xx_config_t *config);

/**
 * @brief 读取芯片信息
 * @param info 芯片信息输出
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_get_device_info(si47xx_info_t *info);

/**
 * @brief 软件复位
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_reset(void);

/**
 * @brief 读取CTS位
 * @return true=就绪, false=未就绪
 */
bool si47xx_check_cts(void);

/**
 * @brief 等待CTS就绪
 * @param timeout_ms 超时毫秒数
 * @return 0=成功, 超时=错误码
 */
si47xx_status_t si47xx_wait_cts(uint32_t timeout_ms);

/*============================================================================
 * 发射机 API
 *============================================================================*/

/**
 * @brief 设置发射频率
 * @param freq_kHz 频率 (kHz), 如 10790 = 107.90 MHz
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_tx_set_frequency(uint32_t freq_kHz);

/**
 * @brief 设置发射功率
 * @param power_dbuv 功率 (dBuV), 范围 77-115
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_tx_set_power(uint8_t power_dbuv);

/**
 * @brief 开启发射
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_tx_enable(void);

/**
 * @brief 关闭发射
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_tx_disable(void);

/**
 * @brief 获取发射状态
 * @param status 状态输出
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_tx_get_status(si47xx_tx_status_t *status);

/*============================================================================
 * 音频 API
 *============================================================================*/

/**
 * @brief 设置音量
 * @param volume 音量 (0-100)
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_audio_set_volume(uint8_t volume);

/**
 * @brief 设置静音
 * @param mute true=静音, false=取消静音
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_audio_set_mute(bool mute);

/**
 * @brief 配置音频参数
 * @param config 音频配置
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_audio_config(const si47xx_audio_config_t *config);

/*============================================================================
 * RDS API (仅 Si4711/13/21)
 *============================================================================*/

/**
 * @brief 开启RDS
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_enable(void);

/**
 * @brief 关闭RDS
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_disable(void);

/**
 * @brief 设置RDS节目信息
 * @param pi_code 节目识别码 (4位BCD)
 * @param ps_name 节目名称 (8字符)
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_set_pi(uint16_t pi_code, const char *ps_name);

/**
 * @brief 设置RDS广播文本
 * @param text 文本内容 (最多64字符)
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_set_text(const char *text);

/**
 * @brief 设置RDS交通公告
 * @param enable true=开启, false=关闭
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_set_ta(bool enable);

/**
 * @brief 设置RDS交通节目标志
 * @param enable true=开启, false=关闭
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_rds_set_tp(bool enable);

/*============================================================================
 * 属性 API
 *============================================================================*/

/**
 * @brief 设置属性
 * @param property 属性ID
 * @param value 属性值
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_set_property(uint16_t property, uint16_t value);

/**
 * @brief 获取属性
 * @param property 属性ID
 * @param value 属性值输出
 * @return 0=成功, 其他=错误码
 */
si47xx_status_t si47xx_get_property(uint16_t property, uint16_t *value);

/*============================================================================
 * 常用属性定义
 *============================================================================*/

/* FM_TX 属性 */
#define PROPERTY_FM_TX_FREQ              0x3701      // 发射频率
#define PROPERTY_FM_TX_POWER_LEVEL      0x3702      // 发射功率
#define PROPERTY_FM_TX_RDS_PI            0x3703      // RDS节目识别码
#define PROPERTY_FM_TX_RDS_PS_MIX        0x3704      // PS混合电平
#define PROPERTY_FM_TX_RDS_PS_MSC_MIX    0x3705      // PS消息混合电平
#define PROPERTY_FM_TX_RDS_DYNAMIC_PS    0x3706      // 动态PS
#define PROPERTY_FM_TX_RDS_TEXT_MIX      0x3707      // 文本混合电平

/* FM 接收属性 */
#define PROPERTY_FM_RX_VOLUME           0x4000      // 音量
#define PROPERTY_FM_RX_RSSI              0x4003      // RSSI阈值

/* 音频属性 */
#define PROPERTY_AUDIO_IO_CONFIG         0x0101      // 音频IO配置

/*============================================================================
 * 命令定义
 *============================================================================*/

/* 命令字节 */
#define CMD_POWER_UP                     0x01
#define CMD_GET_REV                      0x10
#define CMD_SET_PROPERTY                 0x11
#define CMD_GET_PROPERTY                 0x12
#define CMD_GET_INT_STATUS               0x14
#define CMD_FM_TUNE_FREQ                 0x20
#define CMD_FM_SEEK_START                0x21
#define CMD_FM_RDS_STATUS                0x24
#define CMD_FM_RDS_CONFIG                0x25
#define CMD_TX_TUNE_FREQ                 0x30
#define CMD_TX_TUNE_POWER                0x31
#define CMD_TX_TUNE_COMPENSATION         0x32
#define CMD_TX_TUNE_STATUS               0x33
#define CMD_TX_RDS_BUFF                  0x35
#define CMD_TX_RDS_METADATA              0x36

/* 属性组 */
#define PROP_GROUP_GPO_CTL               0x0000
#define PROP_GROUP_INT_CTL               0x0100
#define PROP_GROUP_AUDIO                 0x0200
#define PROP_GROUP_FM_RECEIVE            0x3000
#define PROP_GROUP_FM_TRANS              0x3700
#define PROP_GROUP_TUNE                  0x3C00

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 频率转换为channel
 * @param freq_kHz 频率 (kHz)
 * @return channel值
 */
static inline uint16_t si47xx_freq_to_channel(uint32_t freq_kHz) {
    // 频段起始频率 76000 kHz (76 MHz)
    // 通道间距 10 kHz
    return (freq_kHz - 76000) / 10;
}

/**
 * @brief channel转换为频率
 * @param channel channel值
 * @return 频率 (kHz)
 */
static inline uint32_t si47xx_channel_to_freq(uint16_t channel) {
    return 76000 + (channel * 10);
}

/**
 * @brief 功率值转dBuV
 * @param power 0-7 对应 77-115 dBuV
 * @return dBuV值
 */
static inline uint8_t si47xx_power_to_dbuv(uint8_t power) {
    return 77 + (power * (115 - 77) / 7);
}

/**
 * @brief dBuV转功率索引
 * @param dbuv dBuV值 (77-115)
 * @return 0-7 索引
 */
static inline uint8_t si47xx_dbuv_to_power(uint8_t dbuv) {
    if (dbuv < 77) return 0;
    if (dbuv > 115) return 7;
    return (dbuv - 77) * 7 / (115 - 77);
}

/**
 * @brief 获取错误描述
 * @param status 错误码
 * @return 错误描述字符串
 */
const char* si47xx_error_to_string(si47xx_status_t status);

#endif /* __SI47XX_FM_TRANSMITTER_H__ */