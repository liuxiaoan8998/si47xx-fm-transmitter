# Makefile for Si47xx FM Transmitter Driver

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -O2 -I.
CFLAGS += -DDEBUG

# 目标文件
TARGET = example_fm_transmitter
TEST_TARGET = test_si47xx
LCD_TARGET = test_with_lcd
LCD_SIMPLE_TARGET = lcd_simple_test

# 源文件
SRCS = si47xx_fm_transmitter.c example_fm_transmitter.c
TEST_SRCS = si47xx_fm_transmitter.c test_si47xx.c
LCD_SRCS = si47xx_fm_transmitter.c lcd_error_display.c test_with_lcd.c
LCD_SIMPLE_SRCS = si47xx_fm_transmitter.c lcd_simple_test.c
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)
LCD_OBJS = $(LCD_SRCS:.c=.o)
LCD_SIMPLE_OBJS = $(LCD_SIMPLE_SRCS:.c=.o)

# 默认目标
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(LCD_TARGET): $(LCD_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(LCD_SIMPLE_TARGET): $(LCD_SIMPLE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c si47xx_fm_transmitter.h
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(LCD_OBJS) $(TARGET) $(TEST_TARGET) $(LCD_TARGET)

# 测试
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# LCD 测试
lcd_test: $(LCD_TARGET)
	./$(LCD_TARGET)

# 简化 LCD 测试
lcd_simple: $(LCD_SIMPLE_TARGET)
	./$(LCD_SIMPLE_TARGET)

# 重新编译
rebuild: clean all

# 打印帮助
help:
	@echo "Si47xx FM Transmitter Driver"
	@echo ""
	@echo "Targets:"
	@echo "  all         - 构建示例程序"
	@echo "  test        - 运行单元测试"
	@echo "  lcd_test    - 运行 LCD 错误码测试"
	@echo "  lcd_simple  - 运行简化 LCD 测试 (2 位数字)"
	@echo "  clean       - 清理编译文件"
	@echo "  rebuild     - 重新编译"
	@echo "  help        - 显示帮助"

.PHONY: all clean rebuild help