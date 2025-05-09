/**************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 mian.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : I.MX6U开发板裸机实验13 串口实验
其他	   : 本实验我们学习如何使用I.MX6的串口，实现串口收发数据，了解
		 I.MX6的串口工作原理。
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/15 左忠凯创建
**************************************************************/
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_lcdapi.h"
#include "MCIMX6Y2.h"
#include "system_MCIMX6Y2.h"
#include "bsp_uart.h"
#include "bsp_epittimer.h"
#include <string.h>
#include "stdio.h"

#include "FreeRTOS.h"
#include "task.h"

// 汇编代码运行调试
void put(unsigned int addr)
{
	puts("i am  running \r\n");
	putc((unsigned int)addr>>24);
	putc((unsigned int)addr>>16);
	putc((unsigned int)addr>>8);
	putc((unsigned int)addr);
	puts("i am  running \r\n");
}

#define LCD_FONT_SIZE     24                  // 字号高度为 24 像素
#define LCD_LINE_HEIGHT   28                  // 行高，给每行留 4 像素间隔
#define LCD_TOP_MARGIN    40                  // 顶部预留高度

#define LCD_MAX_LINES     ((700 - LCD_TOP_MARGIN) / LCD_LINE_HEIGHT)  // = 27 行

static uint16_t lcd_current_line = 0;

/* 重定义 puts 函数：串口输出 + LCD 显示 */
void lcd_with_uart(const char *str)
{
    uart_send_string(UART1, str);  // 原始串口输出

    lcd_show_string(10, 40 + lcd_current_line * LCD_LINE_HEIGHT, 240, 16, 16, (char *)str);
    lcd_current_line++;

    if (lcd_current_line >= LCD_MAX_LINES) {
        lcd_current_line = 0;
        lcd_clear(LCD_WHITE);
	}
}

/* LED任务 */
static void led_task(void *pvParameters)
{
    unsigned char state = 0;

    for (;;)
    {
        state = !state;
        led_switch(LED0, state);
        lcd_with_uart("led_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* UART任务 */
static void uart_task(void *pvParameters)
{
    for (;;)
    {
        lcd_with_uart("uart_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*
 * @description	: main函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
	// 初始化中断(一定要最先调用！)     	
	SystemInitIrqTable(); 
	// 初始化系统时钟 			    	              
	imx6u_clkinit();	
	// 初始化延时	             
    delay_init();
	// 使能所有的时钟				          
	clk_enable();
	// 初始化led 				         
	led_init();
	// 初始化串口，波特率115200  					        
	uart_init();				         
	// 66M时钟，660分频，500ms周期
	epit1_init(659, 49999);
	/* LCD 初始化 */
    lcd_init();
	lcd_clear(LCD_WHITE);
	lcd_show_string(10, 10, 240, 24, 24, "串口信息：");
	puts("init over \r\n");

	puts("start to run rtos\r\n");

	if (xTaskCreate(led_task, "led_task", 128, NULL, configMAX_PRIORITIES - 2, NULL) != pdPASS)
    {
		puts("led_task create error \r\n");
        while (1){}
    }

	if (xTaskCreate(uart_task, "uart_task", 128, NULL, configMAX_PRIORITIES - 3, NULL) != pdPASS)
    {
		puts("uart_task create error \r\n");
        while (1){}
    }

	vTaskStartScheduler();               /* 操作系统任务调度            */

	while(1)
	{
		puts("rtos error \r\n");

		delayms(2000);
    }
	
	return 0;
}
