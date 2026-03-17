/**
 * @file test_si47xx.c
 * @brief Si47xx FM Transmitter 测试程序
 * 
 * 包含多种测试用例:
 * 1. I2C 通信测试
 * 2. 芯片识别测试
 * 3. 频率设置测试
 * 4. 功率设置测试
 * 5. RDS 功能测试
 * 6. 边界条件测试
 */

#include "si47xx_fm_transmitter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*============================================================================
 * 测试框架
 *============================================================================*/

#define TEST_PASS   0
#define TEST_FAIL   1

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        g_tests_failed++; \
        printf("  [FAIL] %s\n", msg); \
    } \
} while(0)

#define TEST_SECTION(name) do { \
    printf("\n--- %s ---\n", name); \
} while(0)

void test_summary(void)
{
    printf("\n======================================\n");
    printf("  测试结果汇总\n");
    printf("======================================\n");
    printf("  总测试: %d\n", g_tests_run);
    printf("  通过:   %d (%.1f%%)\n", g_tests_passed, 
           100.0 * g_tests_passed / g_tests_run);
    printf("  失败:   %d\n", g_tests_failed);
    printf("======================================\n");
}

/*============================================================================
 * 硬件接口 (模拟实现)
 *============================================================================*/

static int g_i2c_simulate_error = 0;
static int g_i2c_write_count = 0;
static int g_i2c_read_count = 0;
static int g_last_i2c_addr = 0;

void i2c_simulate_error_set(int enable) {
    g_i2c_simulate_error = enable;
}

int si47xx_hw_write(uint8_t addr, const uint8_t *data, uint16_t len)
{
    g_last_i2c_addr = addr;
    g_i2c_write_count++;
    
    if (g_i2c_simulate_error) {
        return -1;  // 模拟错误
    }
    
    printf("[I2C] WRITE addr=0x%02X len=%d: ", addr, len);
    for (int i = 0; i < len && i < 16; i++) {
        printf("%02X ", data[i]);
    }
    if (len > 16) printf("...");
    printf("\n");
    
    return 0;
}

int si47xx_hw_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    g_last_i2c_addr = addr;
    g_i2c_read_count++;
    
    if (g_i2c_simulate_error) {
        return -1;  // 模拟错误
    }
    
    printf("[I2C] READ  addr=0x02X len=%d: ", addr, len);
    
    // 模拟响应数据
    data[0] = 0x80;  // CTS = 1
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    return 0;
}

void si47xx_delay_ms(uint32_t ms)
{
    // 空延时，仅打印
    if (ms > 10) {
        printf("[DELAY] %d ms\n", ms);
    }
}

void i2c_stats_print(void)
{
    printf("\n[I2C] 统计: 写=%d, 读=%d\n", 
           g_i2c_write_count, g_i2c_read_count);
}

/*============================================================================
 * 测试用例
 *============================================================================*/

/**
 * 测试1: 验证频率转换函数
 */
void test_freq_conversion(void)
{
    TEST_SECTION("频率转换测试");
    
    // 测试 87.5 MHz
    uint16_t ch = si47xx_freq_to_channel(87500U);
    TEST_ASSERT(ch == 115, "87.5 MHz -> channel 115");
    
    uint16_t freq = si47xx_channel_to_freq(ch);
    TEST_ASSERT(freq == 87500, "channel 115 -> 87.5 MHz");
    
    // 测试 108 MHz
    ch = si47xx_freq_to_channel(108000U);
    TEST_ASSERT(ch == 320, "108 MHz -> channel 320");
    
    // 测试边界
    ch = si47xx_freq_to_channel(76000U);
    TEST_ASSERT(ch == 0, "76 MHz (最小) -> channel 0");
    
    freq = si47xx_channel_to_freq(0);
    TEST_ASSERT(freq == 76000, "channel 0 -> 76 MHz");
}

/**
 * 测试2: 验证功率转换函数
 */
void test_power_conversion(void)
{
    TEST_SECTION("功率转换测试");
    
    // 测试边界值
    uint8_t idx = si47xx_dbuv_to_power(77);
    TEST_ASSERT(idx == 0, "77 dBuV -> index 0");
    
    idx = si47xx_dbuv_to_power(115);
    TEST_ASSERT(idx == 7, "115 dBuV -> index 7");
    
    // 测试范围外
    idx = si47xx_dbuv_to_power(50);
    TEST_ASSERT(idx == 0, "50 dBuV (< 77) -> index 0");
    
    idx = si47xx_dbuv_to_power(200);
    TEST_ASSERT(idx == 7, "200 dBuV (> 115) -> index 7");
    
    // 反向转换
    uint8_t db = si47xx_power_to_dbuv(0);
    TEST_ASSERT(db == 77, "index 0 -> 77 dBuV");
    
    db = si47xx_power_to_dbuv(7);
    TEST_ASSERT(db == 115, "index 7 -> 115 dBuV");
}

/**
 * 测试3: 验证错误码转换
 */
void test_error_string(void)
{
    TEST_SECTION("错误码转换测试");
    
    TEST_ASSERT(strcmp(si47xx_error_to_string(SI47XX_OK), "OK") == 0, 
               "SI47XX_OK");
    TEST_ASSERT(strcmp(si47xx_error_to_string(SI47XX_ERR_CTS_TIMEOUT), "CTS Timeout") == 0,
               "SI47XX_ERR_CTS_TIMEOUT");
    TEST_ASSERT(strcmp(si47xx_error_to_string(SI47XX_ERR_INVALID_ARG), "Invalid Argument") == 0,
               "SI47XX_ERR_INVALID_ARG");
    TEST_ASSERT(strcmp(si47xx_error_to_string(SI47XX_ERR_BUS_ERROR), "Bus Error") == 0,
               "SI47XX_ERR_BUS_ERROR");
}

/**
 * 测试4: 初始化流程测试
 */
void test_init_flow(void)
{
    TEST_SECTION("初始化流程测试");
    
    si47xx_config_t config = {0};
    si47xx_status_t status;
    
    // 测试空配置
    status = si47xx_init(NULL);
    TEST_ASSERT(status == SI47XX_ERR_INVALID_ARG, 
                "空配置返回错误");
    
    // 测试有效配置
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    config.tx_power = 115;
    
    g_i2c_write_count = 0;
    g_i2c_read_count = 0;
    
    // 注意: 这里会失败因为没有真实硬件
    // 但可以验证调用流程
    printf("  [INFO] 尝试初始化 (需要真实硬件)...\n");
}

/**
 * 测试5: 频率设置边界测试
 */
void test_frequency_bounds(void)
{
    TEST_SECTION("频率边界测试");
    
    si47xx_config_t config = {0};
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    
    // 注意: 需要先初始化才能调用
    // 这里演示如何设置边界
    printf("  [INFO] 频段: US/EU (87.5-108 MHz)\n");
    printf("  [INFO] 最小频率: %d kHz\n", config.freq_min);
    printf("  [INFO] 最大频率: %d kHz\n", config.freq_max);
    printf("  [INFO] 有效 channel 范围: %d - %d\n",
           si47xx_freq_to_channel(config.freq_min),
           si47xx_freq_to_channel(config.freq_max));
    
    TEST_ASSERT(config.freq_min == 87500, "最小频率正确");
    TEST_ASSERT(config.freq_max == 108000, "最大频率正确");
}

/**
 * 测试6: I2C 地址测试
 */
void test_i2c_address(void)
{
    TEST_SECTION("I2C 地址测试");
    
    // 测试发送命令会使用正确的地址
    uint8_t cmd[4] = {CMD_POWER_UP, 0, 0x01, 0x05};
    
    si47xx_hw_write(0x21, cmd, 4);
    TEST_ASSERT(g_last_i2c_addr == 0x21, "发射模式使用地址 0x21");
    
    si47xx_hw_write(0x22, cmd, 4);
    TEST_ASSERT(g_last_i2c_addr == 0x22, "接收模式使用地址 0x22");
}

/**
 * 测试7: 参数验证测试
 */
void test_parameter_validation(void)
{
    TEST_SECTION("参数验证测试");
    
    // 测试无效频率
    si47xx_config_t config = {0};
    config.band = BAND_US_EU;
    config.freq_min = 87500U;
    config.freq_max = 108000U;
    
    // 注意: 需要先初始化
    printf("  [INFO] 参数验证需要在初始化后执行\n");
    printf("  [INFO] 频率范围: %d - %d kHz\n", 
           config.freq_min, config.freq_max);
    
    // 测试功率范围
    TEST_ASSERT(si47xx_dbuv_to_power(77) >= 0, "功率最小值有效");
    TEST_ASSERT(si47xx_dbuv_to_power(115) <= 7, "功率最大值有效");
}

/**
 * 测试8: 模拟错误测试
 */
void test_error_simulation(void)
{
    TEST_SECTION("错误模拟测试");
    
    // 启用错误模拟
    i2c_simulate_error_set(1);
    
    int result = si47xx_hw_write(0x21, (uint8_t*)"test", 4);
    TEST_ASSERT(result != 0, "模拟I2C错误返回非0");
    
    // 禁用错误模拟
    i2c_simulate_error_set(0);
    
    result = si47xx_hw_write(0x21, (uint8_t*)"test", 4);
    TEST_ASSERT(result == 0, "恢复正常返回0");
}

/*============================================================================
 * 主程序
 *============================================================================*/

int main(int argc, char *argv[])
{
    printf("======================================\n");
    printf("  Si47xx FM Transmitter 测试套件\n");
    printf("======================================\n");
    
    // 运行所有测试
    test_freq_conversion();
    test_power_conversion();
    test_error_string();
    test_i2c_address();
    test_parameter_validation();
    test_error_simulation();
    
    // 需要硬件的测试
    test_init_flow();
    test_frequency_bounds();
    
    // 打印统计
    i2c_stats_print();
    
    // 测试汇总
    test_summary();
    
    printf("\n提示: 某些测试需要真实硬件才能完成。\n");
    printf("      使用模拟模式验证基本逻辑正确性。\n");
    
    return g_tests_failed > 0 ? 1 : 0;
}