/**
 * @file example_fm_transmitter.c
 * @brief Si47xx FM Transmitter 示例程序
 * 
 * 演示基本功能：
 * 1. 初始化芯片
 * 2. 设置发射频率和功率
 * 3. 播放音乐
 * 4. 发送RDS信息 (如果支持)
 * 
 * 硬件连接:
 *   - I2C: SDA -> GPIO0, SCL -> GPIO1
 *   - 复位: RST -> GPIO2
 */

#include "si47xx_fm_transmitter.h"
#include <stdio.h>
#include <unistd.h>

/*============================================================================
 * 硬件接口实现 (需要根据实际硬件修改)
 *============================================================================*/

/**
 * I2C写函数 - 需要用户实现
 */
int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len)
{
    // TODO: 实现I2C写操作
    // 示例:
    // return i2c_write(I2C_PORT, addr, data, len);
    
    printf("[HW] I2C Write: addr=0x%02X, len=%d\n", addr, len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    return 0;
}

/**
 * I2C读函数 - 需要用户实现
 */
int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    // TODO: 实现I2C读操作
    // 示例:
    // return i2c_read(I2C_PORT, addr, data, len);
    
    printf("[HW] I2C Read: addr=0x%02X, len=%d\n", addr, len);
    
    // 模拟CTS响应
    data[0] = 0x80;  // CTS=1
    
    return 0;
}

/**
 * 延时函数
 */
void si47xx_delay_ms(uint32_t ms)
{
    // TODO: 实现精确延时
    // 示例: usleep(ms * 1000);
    printf("[HW] Delay: %d ms\n", ms);
}

/*============================================================================
 * 示例程序
 *============================================================================*/

int main(int argc, char *argv[])
{
    si47xx_status_t status;
    si47xx_config_t config;
    si47xx_info_t   device_info;
    
    printf("======================================\n");
    printf("  Si47xx FM Transmitter Example\n");
    printf("======================================\n\n");
    
    // 1. 配置参数
    printf("[1] 配置参数...\n");
    memset(&config, 0, sizeof(config));
    
    config.band           = BAND_US_EU;        // 美国/欧洲频段
    config.preemphasis    = PREEMPHASIS_USA;    // 75us预加重
    config.freq_min       = 87500;              // 87.5 MHz
    config.freq_max       = 108000;             // 108 MHz
    config.tx_power       = 115;                // 最大功率
    config.audio.volume   = 80;
    config.audio.mute     = false;
    
    // 2. 初始化
    printf("\n[2] 初始化芯片...\n");
    status = si47xx_init(&config);
    if (status != SI47XX_OK) {
        printf("错误: 初始化失败 - %s\n", si47xx_error_to_string(status));
        return -1;
    }
    printf("初始化成功!\n");
    
    // 3. 获取设备信息
    printf("\n[3] 读取设备信息...\n");
    status = si47xx_get_device_info(&device_info);
    if (status == SI47XX_OK) {
        printf("  芯片型号: %d\n", device_info.device);
        printf("  固件版本: %d.%d\n", 
               device_info.firmware_major, 
               device_info.firmware_minor);
    }
    
    // 4. 设置发射频率
    printf("\n[4] 设置发射频率 107.90 MHz...\n");
    status = si47xx_tx_set_frequency(107900);
    if (status != SI47XX_OK) {
        printf("错误: 设置频率失败 - %s\n", si47xx_error_to_string(status));
        return -1;
    }
    printf("频率设置成功!\n");
    
    // 5. 设置发射功率
    printf("\n[5] 设置发射功率 115 dBuV...\n");
    status = si47xx_tx_set_power(115);
    if (status != SI47XX_OK) {
        printf("错误: 设置功率失败 - %s\n", si47xx_error_to_string(status));
        return -1;
    }
    printf("功率设置成功!\n");
    
    // 6. 设置音量
    printf("\n[6] 设置音量 80%%...\n");
    status = si47xx_audio_set_volume(80);
    if (status != SI47XX_OK) {
        printf("错误: 设置音量失败 - %s\n", si47xx_error_to_string(status));
    }
    printf("音量设置成功!\n");
    
    // 7. 开启发射
    printf("\n[7] 开启发射...\n");
    status = si47xx_tx_enable();
    if (status != SI47XX_OK) {
        printf("错误: 开启发射失败 - %s\n", si47xx_error_to_string(status));
        return -1;
    }
    printf("发射已开启!\n");
    
    // 8. 启用RDS (仅Si4711/13/21支持)
    printf("\n[8] 启用RDS...\n");
    status = si47xx_rds_enable();
    if (status != SI47XX_OK) {
        printf("RDS不可用或不支持: %s\n", si47xx_error_to_string(status));
    } else {
        // 设置RDS节目信息
        si47xx_rds_set_pi(0x1234, "TEST_FM");
        si47xx_rds_set_tp(false);
        si47xx_rds_set_ta(false);
        
        // 发送广播文本
        si47xx_rds_set_text("Si47xx FM Transmitter Test - 107.90 MHz");
        printf("RDS配置完成!\n");
    }
    
    printf("\n======================================\n");
    printf("  FM 发射器已启动!\n");
    printf("  频率: 107.90 MHz\n");
    printf("  功率: 115 dBuV\n");
    printf("======================================\n");
    
    // 保持运行
    while (1) {
        si47xx_delay_ms(1000);
    }
    
    return 0;
}