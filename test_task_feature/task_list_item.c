/**************************************************************

文件名		: mian.c
作者	   : sino
版本	   : V1.0
描述	   : FREERTOS移植后列表和列表项使用
其他	   : 使用任务创建和列表项验证插入和删除的实验效果。
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
#define START_STK_SIZE 128
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

/* 按键任务句柄和函数 */

TaskHandle_t KeyTask_Handler;
void key_task(void *pvParameters);

/* 灯控任务句柄和函数 */

void list_task(void *pvParameters);
TaskHandle_t ListTask_Handler;

List_t TestList;
ListItem_t ListItem1;
ListItem_t ListItem2;
ListItem_t ListItem3;

List_t TestList;
ListItem_t ListItem1;
ListItem_t ListItem2;
ListItem_t ListItem3;

/* UART任务 */
static void uart_task(void *pvParameters)
{
    for (;;)
    {
        // puts("uart_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void start_task(void *pvParameters){
	taskENTER_CRITICAL();
	//创建任务task1
	xTaskCreate(
		(TaskFunction_t			)key_task,
		(const char*			)"key_task",
		(u16					)128,	
		(void*					)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-4,
		(TaskHandle_t*			)&KeyTask_Handler

	);
	//创建任务task2
	xTaskCreate(
		(TaskFunction_t			)list_task,
		(const char*			)"list_task",
		(u16					)128,	
		(void*					)NULL,	
		(UBaseType_t			)configMAX_PRIORITIES-3,
		(TaskHandle_t*			)&ListTask_Handler

	);
	vTaskDelete(StartTask_Handler);
	// 退出临界区，恢复中断和调度
    taskEXIT_CRITICAL();
}

/* KEY任务 */
void key_task(void *pvParameters)
{
	unsigned int keyvalue;

	while (1)
	{	
		keyvalue=key_getvalue();

		keyvalue = !keyvalue;
		led_switch(LED0, keyvalue);
		// printf("led_sum's running times:%d",led_sum);
		vTaskDelay(pdMS_TO_TICKS(100));

	}
	
}


void list_task(void *pvParamter)
{
	//第一步初始化列表和列表项
	vListInitialise(&TestList);
	vListInitialiseItem(&ListItem1);
	vListInitialiseItem(&ListItem2);
	vListInitialiseItem(&ListItem3);

	ListItem1.xItemValue = 40;
	ListItem2.xItemValue = 60;
	ListItem3.xItemValue = 50;

	//第二部打印列表和其他列表项地址
	printf("/**********列表和列表项地址************/\r\n");
	printf("项目						地址\r\n");
	printf("TestList						%#x	\r\n",(int)&TestList);
	printf("TestList->pxIdex					%#x	\r\n",(int)TestList.pxIndex);
	printf("TestList->xListEnd 					%#x	\r\n",(int)(&TestList.xListEnd));
	printf("ListItem1					%#x	\r\n",(int)&ListItem1);
	printf("ListItem2					%#x	\r\n",(int)&ListItem2);
	printf("ListItem3					%#x	\r\n",(int)&ListItem3);
	printf("/***************结束*****************/\r\n");
	printf("按下KEY键继续!\r\n\r\n\r\n");
	while(key_getvalue()==LED0)delayms(10);

	//第三步插入ListItem1到列表中
	vListInsert(&TestList,&ListItem1);
	printf("/**********添加列表项List Item1************/\r\n");
	printf("项目						地址\r\n");
	printf("TestList					%#x	\r\n",(int)&TestList);
	printf("TestList->xListEnd->pxNext			%#x	\r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext				%#x	\r\n",(int)(ListItem1.pxNext));
	printf("/*********前后向连接分割线************/\r\n");

	printf("TestList->xListEnd->pxPrevious	%#x	\r\n",(int)(TestList.xListEnd.pxPrevious));
	printf("ListItem1->pxPrevious	%#x	\r\n",(int)(ListItem1.pxPrevious));
	printf("/***************结束*****************/\r\n");
	printf("按下KEY键继续!\r\n\r\n\r\n");
	while(key_getvalue()==LED0)delayms(10);
	//第4步插入ListItem2到列表中


	vListInsert(&TestList,&ListItem2);
	printf("/**********添加列表项List Item2************/\r\n");
	printf("项目					地址\r\n");
	printf("TestList->xListEnd->pxNext			%#x	\r\n",(int)(TestList.xListEnd.pxNext));

	printf("ListItem1->pxNext				%#x	\r\n",(int)(ListItem1.pxNext));
	printf("ListItem2->pxNext				%#x	\r\n",(int)(ListItem2.pxNext));
	printf("/*********前后向连接分割线************/\r\n");

	printf("ListItem1->pxPrevious	%#x	\r\n",(int)(ListItem1.pxPrevious));
	printf("ListItem2->pxPrevious	%#x	\r\n",(int)(ListItem2.pxPrevious));
	printf("/***************结束*****************/\r\n");
	printf("按下KEY键继续!\r\n\r\n\r\n");
	while(key_getvalue()==LED0)delayms(10);

	//第5步插入ListItem3到列表中

	vListInsert(&TestList,&ListItem3);
	printf("/**********添加列表项List Item3************/\r\n");
	printf("项目					地址\r\n");
	printf("TestList->xListEnd->pxNext			%#x	\r\n",(int)(TestList.xListEnd.pxNext));

	printf("ListItem1->pxNext				%#x	\r\n",(int)(ListItem1.pxNext));
	printf("ListItem3->pxNext				%#x	\r\n",(int)(ListItem3.pxNext));
	printf("ListItem2->pxNext				%#x	\r\n",(int)(ListItem2.pxNext));
	printf("/*********前后向连接分割线************/\r\n");

	printf("ListItem1->pxPrevious	%#x	\r\n",(int)(ListItem1.pxPrevious));
	printf("ListItem3->pxPrevious	%#x	\r\n",(int)(ListItem3.pxPrevious));
	printf("ListItem2->pxPrevious	%#x	\r\n",(int)(ListItem2.pxPrevious));
	printf("/***************结束*****************/\r\n");
	printf("按下KEY键继续!\r\n\r\n\r\n");
	while(key_getvalue()==LED0)delayms(10);

	//第6步删除ListItem2列表中
	uxListRemove(&ListItem2);
	printf("/**********删除ListItem2列表中************/\r\n");
	printf("项目					地址\r\n");
	printf("TestList->xListEnd->pxNext			%#x	\r\n",(int)(TestList.xListEnd.pxNext));
	printf("ListItem1->pxNext				%#x	\r\n",(int)(ListItem1.pxNext));
	printf("ListItem3->pxNext				%#x	\r\n",(int)(ListItem2.pxNext));
	printf("/*********前后向连接分割线************/\r\n");

	printf("ListItem1->pxPrevious	%#x	\r\n",(int)(ListItem1.pxPrevious));
	printf("ListItem3->pxPrevious	%#x	\r\n",(int)(ListItem3.pxPrevious));
	printf("/***************结束*****************/\r\n");
	printf("按下KEY键继续!\r\n\r\n\r\n");
	while(key_getvalue()==LED0)delayms(10);
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
	// epit1_init(659, 49999);

	puts("init over \r\n");
	puts("start to run rtos\r\n");



	xTaskCreate((TaskFunction_t)	start_task,
				(const char*)		"start_task",
				(const u16)	START_STK_SIZE,
				(void		*)		NULL,
				(UBaseType_t )		configMAX_PRIORITIES,
				(TaskHandle_t)		&StartTask_Handler);
	
	xTaskCreate((TaskFunction_t)	uart_task,
			(const char*)		"uart_task",
			(const u16)			128,
			(void		*)		NULL,
			(UBaseType_t )		configMAX_PRIORITIES-1,
			(TaskHandle_t)		NULL);
	
	vTaskStartScheduler();               /* 操作系统任务调度  */
	while (1)
	{
		delayms(100);
	}
	
	return 0;
}
