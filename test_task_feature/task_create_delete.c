/**************************************************************

文件名	: 	 mian.c
作者	   : 
版本	   : V1.0
描述	   : FREERTOS移植后任务创建和删除实例
其他	   : 使用串口和LCD具体显示了，任务创建步骤、人物删除、以及临界段代码使用。
            保证子任务完整运行，以及任务阻塞后的调度。
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

TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

TaskHandle_t Task1_Handler;
void lcd1_task(void *pvParameters);

TaskHandle_t Task2_Handler;
void lcd2_task(void *pvParameters);


/* UART任务 */
static void uart_task(void *pvParameters)
{
    for (;;)
    {
        puts("uart_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
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
        puts("led_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void start_task(void *pvParameters){
	taskENTER_CRITICAL();
	//创建任务task1
	xTaskCreate(
		(TaskFunction_t	)lcd1_task,
		(const char*	)"lcd1_task",
		(u16			)128,	
		(void*		)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-3,
		(TaskHandle_t*			)&Task1_Handler

	);
	//创建任务task2
	xTaskCreate(
		(TaskFunction_t	)lcd2_task,
		(const char*	)"lcd2_task",
		(u16			)128,	
		(void*		)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-4,
		(TaskHandle_t*			)&Task2_Handler

	);
	vTaskDelete(StartTask_Handler);
	// 退出临界区，恢复中断和调度
    taskEXIT_CRITICAL();
}
void lcd1_task(void *pvParameters){

	uint8_t task1_num = 0;

	lcd_draw_rectangle(5,100,115,200);
	lcd_drawline(5,130,115,130);
	lcd_show_string(6,111,110,16,16,"lcd_Task1 Run:000");
	while (1)
	{	
		task1_num++;
		printf("任务1已经执行%d\r\n",task1_num);

		if (task1_num == 11)
		{
			vTaskDelete(Task2_Handler);
			printf("任务1删除了任务2\r\n");
		}
		lcd_fill(5,100,115,200,LCD_BLACK);
		
		lcd_shownum(86,111,task1_num,3,16);
		
		vTaskDelay(pdMS_TO_TICKS(500));
		/* code */
	}
	
}
void lcd2_task(void *pvParameters){

	uint8_t task2_num = 0;
	lcd_draw_rectangle(125, 100, 235, 200);  // 保持宽度一致
	lcd_drawline(125, 130, 235, 130);

	lcd_show_string(126,111,110,16,16,"lcd_Task2 Run:000");
	while (1)
	{	
		task2_num++;
		printf("任务2已经执行%d\r\n",task2_num);
        lcd_fill(125, 100, 235, 200, LCD_BLUE); // 不要覆盖 task1 区域
        lcd_shownum(206, 111, task2_num, 3, 16);  // 避免覆盖 task1 区域
		vTaskDelay(pdMS_TO_TICKS(250));
		/* code */
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
	//lcd init
	lcd_init();
	lcd_clear(LCD_WHITE);
	puts("init over \r\n");

	puts("start to run rtos\r\n");




	if (xTaskCreate(uart_task, "uart_task", 128, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS)
    {
		puts("uart_task create error \r\n");
        while (1){}
    }
	if (xTaskCreate(led_task, "led_task", 128, NULL, configMAX_PRIORITIES - 2, NULL) != pdPASS)
    {
		puts("led_task create error \r\n");
        while (1){}
    }
	if (xTaskCreate(start_task, "start_task",128,NULL, configMAX_PRIORITIES - 3,&StartTask_Handler) != pdPASS)
	{
		puts("start_task create error \r\n");
        while (1){}
	}
	
	vTaskStartScheduler();               /* 操作系统任务调度  */

	while(1)
	{
		puts("rtos error \r\n");

		delayms(2000);
    }
	
	return 0;
}
