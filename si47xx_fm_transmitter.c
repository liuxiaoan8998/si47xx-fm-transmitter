/**
 * @file si47xx_fm_transmitter.c
 * @brief Silicon Labs Si47xx FM Transmitter Driver Implementation
 * @version 1.0
 */

#include "si47xx_fm_transmitter.h"
#include <stdio.h>

/*============================================================================
 * 私有变量
 *============================================================================*/

static si47xx_info_t      g_device_info;
static si47xx_config_t    g_config;
static bool               g_initialized = false;
static bool               g_tx_enabled = false;
static bool               g_rds_enabled = false;

/* I2C 地址 */
#define SI47XX_I2C_ADDR_TX   0x21    // Transmitter
#define SI47XX_I2C_ADDR_RX   0x22    // Receiver

/*============================================================================
 * 私有函数
 *============================================================================*/

/**
 * @brief 发送命令并等待CTS
 */
static si47xx_status_t si47xx_send_cmd(const uint8_t *cmd, uint8_t cmd_len)
{
    si47xx_status_t status;
    uint8_t         resp[1];
    uint32_t        timeout = SI47XX_CTS_TIMEOUT;
    
    // 发送命令
    if (si47xx_hw_write(SI47XX_I2C_ADDR_TX, cmd, cmd_len) != 0) {
        return SI47XX_ERR_BUS_ERROR;
    }
    
    // 等待CTS
    while (timeout > 0) {
        si47xx_delay_ms(1);
        
        if (si47xx_hw_read(SI47XX_I2C_ADDR_TX, resp, 1) != 0) {
            return SI47XX_ERR_BUS_ERROR;
        }
        
        if (resp[0] & 0x80) {  // CTS位为1
            return SI47XX_OK;
        }
        timeout--;
    }
    
    return SI47XX_ERR_CTS_TIMEOUT;
}

/**
 * @brief 发送命令并读取响应
 */
static si47xx_status_t si47xx_send_cmd_with_resp(
    const uint8_t *cmd, uint8_t cmd_len,
    uint8_t *resp, uint8_t resp_len)
{
    si47xx_status_t status;
    
    status = si47xx_send_cmd(cmd, cmd_len);
    if (status != SI47XX_OK) {
        return status;
    }
    
    // 读取响应
    if (si47xx_hw_read(SI47XX_I2C_ADDR_TX, resp, resp_len) != 0) {
        return SI47XX_ERR_BUS_ERROR;
    }
    
    return SI47XX_OK;
}

/*============================================================================
 * 核心 API 实现
 *============================================================================*/

si47xx_status_t si47xx_reset(void)
{
    // 硬件复位通常通过GPIO控制
    // 这里假设复位后需要等待芯片就绪
    si47xx_delay_ms(200);
    return SI47XX_OK;
}

bool si47xx_check_cts(void)
{
    uint8_t resp[1];
    
    if (si47xx_hw_read(SI47XX_I2C_ADDR_TX, resp, 1) != 0) {
        return false;
    }
    
    return (resp[0] & 0x80) != 0;
}

si47xx_status_t si47xx_wait_cts(uint32_t timeout_ms)
{
    uint32_t start = 0;
    
    while (start < timeout_ms) {
        if (si47xx_check_cts()) {
            return SI47XX_OK;
        }
        si47xx_delay_ms(1);
        start++;
    }
    
    return SI47XX_ERR_CTS_TIMEOUT;
}

si47xx_status_t si47xx_init(const si47xx_config_t *config)
{
    uint8_t cmd[10];
    uint8_t resp[15];
    si47xx_status_t status;
    
    if (config == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    // 复制配置
    memcpy(&g_config, config, sizeof(si47xx_config_t));
    
    // 复位芯片
    si47xx_reset();
    
    // 发送 POWER_UP 命令
    cmd[0] = CMD_POWER_UP;
    cmd[1] = 0;  // CTSE 模式
    cmd[2] = 0x01; // 功能: FM 发射
    cmd[3] = 0x05; // I2C 模式
    
    status = si47xx_send_cmd_with_resp(cmd, 4, resp, 15);
    if (status != SI47XX_OK) {
        return status;
    }
    
    // 验证响应
    if ((resp[0] & 0x40) == 0) {  // STATUS CTS
        return SI47XX_ERR_CTS_TIMEOUT;
    }
    
    // 检查芯片响应
    if (resp[1] != 0) {  // ERR 响应
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    // 等待内部初始化完成
    si47xx_delay_ms(300);
    
    // 获取设备信息
    si47xx_get_device_info(&g_device_info);
    
    g_initialized = true;
    
    return SI47XX_OK;
}

si47xx_status_t si47xx_get_device_info(si47xx_info_t *info)
{
    uint8_t cmd[2];
    uint8_t resp[15];
    si47xx_status_t status;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    if (info == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    cmd[0] = CMD_GET_REV;
    
    status = si47xx_send_cmd_with_resp(cmd, 1, resp, 15);
    if (status != SI47XX_OK) {
        return status;
    }
    
    // 解析版本信息
    info->part_number      = resp[1];
    info->firmware_major   = resp[2];
    info->firmware_minor  = resp[3];
    info->patch_major     = (resp[4] << 8) | resp[5];
    info->patch_minor     = (resp[6] << 8) | resp[7];
    info->patch_id        = (resp[8] << 8) | resp[9];
    
    // 识别芯片型号
    switch (info->part_number) {
        case 0x10: info->device = SI4710; break;
        case 0x11: info->device = SI4711; break;
        case 0x12: info->device = SI4712; break;
        case 0x13: info->device = SI4713; break;
        case 0x20: info->device = SI4720; break;
        case 0x21: info->device = SI4721; break;
        default:   info->device = SI47XX_UNKNOWN; break;
    }
    
    return SI47XX_OK;
}

/*============================================================================
 * 发射机 API 实现
 *============================================================================*/

si47xx_status_t si47xx_tx_set_frequency(uint32_t freq_kHz)
{
    uint8_t cmd[6];
    si47xx_status_t status;
    uint16_t channel;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    // 验证频率范围
    if (freq_kHz < g_config.freq_min || freq_kHz > g_config.freq_max) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    // 转换为channel
    channel = si47xx_freq_to_channel(freq_kHz);
    
    cmd[0] = CMD_TX_TUNE_FREQ;
    cmd[1] = 0;  // 保留
    cmd[2] = (channel >> 8) & 0xFF;  // 频率高位
    cmd[3] = channel & 0xFF;         // 频率低位
    
    status = si47xx_send_cmd(cmd, 4);
    if (status != SI47XX_OK) {
        return status;
    }
    
    // 等待调谐完成 (约50ms)
    si47xx_delay_ms(60);
    
    return si47xx_wait_cts(SI47XX_CTS_TIMEOUT);
}

si47xx_status_t si47xx_tx_set_power(uint8_t power_dbuv)
{
    uint8_t cmd[6];
    si47xx_status_t status;
    uint8_t power_idx;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    // 限制功率范围
    if (power_dbuv < 77) power_dbuv = 77;
    if (power_dbuv > 115) power_dbuv = 115;
    
    power_idx = si47xx_dbuv_to_power(power_dbuv);
    
    cmd[0] = CMD_TX_TUNE_POWER;
    cmd[1] = 0;  // 保留
    cmd[2] = power_idx;   // 功率索引 0-7
    cmd[3] = 0;  // 保留 (TX_COMP)
    
    status = si47xx_send_cmd(cmd, 4);
    if (status != SI47XX_OK) {
        return status;
    }
    
    // 等待完成
    si47xx_delay_ms(60);
    
    return si47xx_wait_cts(SI47XX_CTS_TIMEOUT);
}

si47xx_status_t si47xx_tx_enable(void)
{
    si47xx_status_t status;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    // 设置默认频率和功率（发射器自动启用）
    status = si47xx_tx_set_frequency(87500U);  // 默认 87.5 MHz
    if (status != SI47XX_OK) return status;
    
    status = si47xx_tx_set_power(115);  // 默认最大功率
    if (status != SI47XX_OK) return status;
    
    g_tx_enabled = true;
    
    return SI47XX_OK;
}

si47xx_status_t si47xx_tx_disable(void)
{
    // 通过设置功率为0来禁用发射
    si47xx_tx_set_power(0);
    g_tx_enabled = false;
    
    return SI47XX_OK;
}

si47xx_status_t si47xx_tx_get_status(si47xx_tx_status_t *status)
{
    uint8_t cmd[2];
    uint8_t resp[8];
    si47xx_status_t res;
    
    if (status == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    cmd[0] = CMD_TX_TUNE_STATUS;
    cmd[1] = 0x01;  // 读取完整状态
    
    res = si47xx_send_cmd_with_resp(cmd, 2, resp, 8);
    if (res != SI47XX_OK) {
        return res;
    }
    
    // 解析状态
    status->frequency   = ((resp[3] << 8) | resp[4]) * 10 + 76000;
    status->tx_power   = resp[5];
    status->tx_enabled = g_tx_enabled;
    status->rds_enabled = g_rds_enabled;
    
    return SI47XX_OK;
}

/*============================================================================
 * 音频 API 实现
 *============================================================================*/

si47xx_status_t si47xx_audio_set_volume(uint8_t volume)
{
    si47xx_status_t status;
    
    if (volume > 100) volume = 100;
    
    // 音量属性范围 0-63
    uint16_t vol_prop = (volume * 63) / 100;
    
    status = si47xx_set_property(PROPERTY_FM_RX_VOLUME, vol_prop);
    
    if (status == SI47XX_OK) {
        g_config.audio.volume = volume;
    }
    
    return status;
}

si47xx_status_t si47xx_audio_set_mute(bool mute)
{
    uint16_t prop_value;
    si47xx_status_t status;
    
    // 属性 0x0101: GPO2 音频使能控制
    prop_value = mute ? 0x0000 : 0x0001;
    
    status = si47xx_set_property(PROPERTY_AUDIO_IO_CONFIG, prop_value);
    
    if (status == SI47XX_OK) {
        g_config.audio.mute = mute;
    }
    
    return status;
}

si47xx_status_t si47xx_audio_config(const si47xx_audio_config_t *config)
{
    si47xx_status_t status;
    
    if (config == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    status = si47xx_audio_set_volume(config->volume);
    if (status != SI47XX_OK) return status;
    
    status = si47xx_audio_set_mute(config->mute);
    if (status != SI47XX_OK) return status;
    
    memcpy(&g_config.audio, config, sizeof(si47xx_audio_config_t));
    
    return SI47XX_OK;
}

/*============================================================================
 * 属性 API 实现
 *============================================================================*/

si47xx_status_t si47xx_set_property(uint16_t property, uint16_t value)
{
    uint8_t cmd[6];
    si47xx_status_t status;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    cmd[0] = CMD_SET_PROPERTY;
    cmd[1] = 0;  // 保留
    cmd[2] = (property >> 8) & 0xFF;  // 属性高位
    cmd[3] = property & 0xFF;          // 属性低位
    cmd[4] = (value >> 8) & 0xFF;      // 值高位
    cmd[5] = value & 0xFF;             // 值低位
    
    status = si47xx_send_cmd(cmd, 6);
    
    if (status == SI47XX_OK) {
        si47xx_delay_ms(10);
    }
    
    return status;
}

si47xx_status_t si47xx_get_property(uint16_t property, uint16_t *value)
{
    uint8_t cmd[4];
    uint8_t resp[6];
    si47xx_status_t status;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    if (value == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    cmd[0] = CMD_GET_PROPERTY;
    cmd[1] = 0;  // 保留
    cmd[2] = (property >> 8) & 0xFF;
    cmd[3] = property & 0xFF;
    
    status = si47xx_send_cmd_with_resp(cmd, 4, resp, 6);
    if (status != SI47XX_OK) {
        return status;
    }
    
    *value = (resp[3] << 8) | resp[4];
    
    return SI47XX_OK;
}

/*============================================================================
 * RDS API 实现
 *============================================================================*/

si47xx_status_t si47xx_rds_enable(void)
{
    uint8_t cmd[4];
    si47xx_status_t status;
    
    if (!g_initialized) {
        return SI47XX_ERR_DEVICE_NOT_READY;
    }
    
    // 通过设置FM_RDS_ENABLE属性
    status = si47xx_set_property(0x3708, 0x0001);
    if (status != SI47XX_OK) return status;
    
    // 配置RDS属性
    status = si47xx_set_property(0x3704, 0x0001);  // PS混合
    if (status != SI47XX_OK) return status;
    
    status = si47xx_set_property(0x3705, 0x0001);  // MSG混合
    
    g_rds_enabled = (status == SI47XX_OK);
    
    return status;
}

si47xx_status_t si47xx_rds_disable(void)
{
    si47xx_status_t status;
    
    status = si47xx_set_property(0x3708, 0x0000);
    g_rds_enabled = false;
    
    return status;
}

si47xx_status_t si47xx_rds_set_pi(uint16_t pi_code, const char *ps_name)
{
    si47xx_status_t status;
    
    if (ps_name == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    // 设置PI码
    status = si47xx_set_property(PROPERTY_FM_TX_RDS_PI, pi_code);
    if (status != SI47XX_OK) return status;
    
    // 设置PS名称需要通过RDS缓冲区命令
    // 这里简化处理，实际需要使用 CMD_TX_RDS_METADATA
    
    g_config.audio.volume = 0; // 标记已配置
    
    return SI47XX_OK;
}

si47xx_status_t si47xx_rds_set_text(const char *text)
{
    if (text == NULL) {
        return SI47XX_ERR_INVALID_ARG;
    }
    
    // 需要通过 CMD_TX_RDS_BUFF 命令发送
    // 这里简化处理，实际需要实现完整RDS文本传输
    
    (void)text;
    
    return SI47XX_OK;
}

si47xx_status_t si47xx_rds_set_ta(bool enable)
{
    return si47xx_set_property(0x370A, enable ? 0x0001 : 0x0000);
}

si47xx_status_t si47xx_rds_set_tp(bool enable)
{
    return si47xx_set_property(0x370B, enable ? 0x0001 : 0x0000);
}

/*============================================================================
 * 辅助函数实现
 *============================================================================*/

const char* si47xx_error_to_string(si47xx_status_t status)
{
    switch (status) {
        case SI47XX_OK:                      return "OK";
        case SI47XX_ERR_CTS_TIMEOUT:         return "CTS Timeout";
        case SI47XX_ERR_INVALID_ARG:         return "Invalid Argument";
        case SI47XX_ERR_BUS_ERROR:            return "Bus Error";
        case SI47XX_ERR_DEVICE_NOT_READY:     return "Device Not Ready";
        case SI47XX_ERR_INVALID_DEVICE:       return "Invalid Device";
        case SI47XX_ERR_TUNE_FAILED:          return "Tune Failed";
        case SI47XX_ERR_SEEK_FAILED:          return "Seek Failed";
        default:                              return "Unknown Error";
    }
}