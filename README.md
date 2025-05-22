# imx6u_freertos_uart_lcd_demo
A bare-metal demo for i.MX6U using FreeRTOS with UART and LCD display


最初来源论坛：http://47.111.11.73/thread-343800-1-7.html

# RTOS任务功能测试说明

本目录包含多个RTOS任务相关功能的测试示例，主要验证了FreeRTOS中任务的创建、删除、挂起、恢复以及列表项操作等功能。

## 任务基础功能测试 (init_test.c)

这是一个基础的RTOS任务创建和运行的示例，演示了：

- 系统初始化流程
- 创建两个基本任务（LED控制和UART输出）
- 任务调度器启动
- 验证任务正常运行

## 任务创建与删除测试 (task_create_delete.c)

该示例验证了任务的创建和删除功能：

- 创建多个不同优先级的任务（uart_task, led_task, lcd1_task, lcd2_task）
- 使用临界区保护任务创建过程
- 通过`vTaskDelete()`动态删除任务
- 在任务1中执行到特定条件（执行次数达到11次）时删除任务2
- 通过LCD显示和串口输出直观展示任务的运行和删除过程

## 任务挂起与恢复测试 (task_suspend_resume.c)

该示例验证了任务的挂起和恢复功能：

- 创建按键控制任务和LED控制任务
- 通过按键0触发`vTaskSuspend()`挂起LED任务
- 通过按键1触发`vTaskResume()`恢复LED任务运行
- 直观验证任务挂起时会暂停执行，恢复后继续运行
- 任务状态变化通过串口输出反馈

## 任务列表项操作测试 (task_list_item.c)

该示例验证了FreeRTOS内部任务列表和列表项的操作：

- 列表和列表项的初始化（`vListInitialise`, `vListInitialiseItem`）
- 设置列表项优先级值（`xItemValue`）
- 展示列表和列表项的内存地址结构
- 向列表中插入列表项（`vListInsert`）
- 从列表中删除列表项（`uxListRemove`）
- 通过串口输出验证列表操作前后的内存地址关系

## 使用方法

1. 选择特定测试文件编译运行
2. 观察串口输出了解任务运行状态
3. 对于挂起恢复测试，通过按键控制任务状态
4. 对于列表项测试，按照提示按键逐步查看列表操作结果

## 注意事项

- 所有测试基于FreeRTOS实现
- 测试中使用了LED、LCD和串口等外设进行状态显示
- 任务优先级设置遵循FreeRTOS的配置规则
- 部分测试使用临界区保护关键代码段执行 