/**************************************************************

文件名	: 	 mian.c
作者	   : 
版本	   : V1.0
描述	   : FREERTOS移植后任务挂起和解挂
其他	   : 使用串口和LCD具体显示了，任务创建步骤、人物删除、以及临界段代码使用。
            保证子任务完整运行，以及任务阻塞后的调度。
**************************************************************/
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_lcdapi.h"
#include "bsp_uart.h"
#include "bsp_epittimer.h"
#include "bsp_key.h"
#include "MCIMX6Y2.h"
#include "system_MCIMX6Y2.h"

#include <string.h>
#include "stdio.h"

#include "FreeRTOS.h"
#include "task.h"

/* 主任务句柄和函数 */
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

/* 按键任务句柄和函数 */
TaskHandle_t KeyTask_Handler;
void key_task(void *pvParameters);

/* 灯控任务句柄和函数 */
TaskHandle_t LedTask_Handler;
void led_task(void *pvParameters);


/* UART任务 */
static void uart_task(void *pvParameters)
{
    for (;;)
    {
        puts("uart_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void start_task(void *pvParameters){
	taskENTER_CRITICAL();
	//创建任务task1
	xTaskCreate(
		(TaskFunction_t	)key_task,
		(const char*	)"key_task",
		(u16			)128,	
		(void*		)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-4,
		(TaskHandle_t*			)&KeyTask_Handler

	);
	//创建任务task2
	xTaskCreate(
		(TaskFunction_t	)led_task,
		(const char*	)"led_task",
		(u16			)128,	
		(void*		)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-3,
		(TaskHandle_t*			)&LedTask_Handler

	);
	vTaskDelete(StartTask_Handler);
	// 退出临界区，恢复中断和调度
    taskEXIT_CRITICAL();
}

/* KEY任务 */
static void key_task(void *pvParameters)
{
	unsigned int keyvalue;

	while (1)
	{
		keyvalue=key_getvalue();
		switch (keyvalue)
		{
		case 0:
			vTaskSuspend(LedTask_Handler);
			printf("挂起灯控的运行！\r\n");
			break;
		case 1:
			vTaskResume(LedTask_Handler);
			printf("恢复灯控的运行！\r\n");
			break;
		}

		vTaskDelay(50);
	}
	
}

/* LED任务 */
static void led_task(void *pvParameters)
{
	unsigned char led_state = 0;
	unsigned int led_sum = 0;
   while(1){
		led_sum++;
		led_state = !led_state;
		led_switch(LED0, led_state);
        printf("led_sum's running times:%d",led_sum);
        vTaskDelay(pdMS_TO_TICKS(500));
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
	//初始化按键				+
	key_init();           
	// 66M时钟，660分频，500ms周期
	epit1_init(659, 49999);

	puts("init over \r\n");
	puts("start to run rtos\r\n");




	if (xTaskCreate(uart_task, "uart_task", 128, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS)
    {
		puts("uart_task create error \r\n");
        while (1){}
    }

	if (xTaskCreate(start_task, "start_task",128,NULL, configMAX_PRIORITIES -2,&StartTask_Handler) != pdPASS)
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
