/***************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 start.s
作者	   : 左忠凯
版本	   : V1.0
描述	   : ZERO-I.MX6UL/I.MX6ULL开发板启动文件，完成C环境初始化，
		 C环境初始化完成以后跳转到C代码。
其他	   : 无
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/3 左忠凯修改
		 V2.0 2019/1/4 左忠凯修改
		 添加中断相关定义
**************************************************************/

.global _start  				/* 全局标号 */
.extern put
.extern FreeRTOS_IRQ_Handler
.extern FreeRTOS_SWI_Handler

/*
 * 描述：	_start函数，首先是中断向量表的创建
 * 参考文档:ARM Cortex-A(armV7)编程手册V4.0.pdf P42，3 ARM Processor Modes and Registers（ARM处理器模型和寄存器）
 * 		   ARM Cortex-A(armV7)编程手册V4.0.pdf P165 11.1.1 Exception priorities(异常)
 */
_start:
	ldr pc, =Reset_Handler		          /* 复位中断 					    */	
	ldr pc, =Undefined_Handler	          /* 未定义中断 					*/
	ldr pc, =FreeRTOS_SWI_Handler		  /* SVC(Supervisor)中断 		   */
	ldr pc, =PrefAbort_Handler	          /* 预取终止中断 					*/
	ldr pc, =DataAbort_Handler	          /* 数据终止中断 					*/
	ldr	pc, =NotUsed_Handler	          /* 未使用中断					   */
	ldr pc, =FreeRTOS_IRQ_Handler		  /* IRQ中断 					  */
	ldr pc, =FIQ_Handler		          /* FIQ(快速中断)未定义中断 	     */

/* 复位中断 */	
Reset_Handler:

	cpsid i						/* 关闭全局中断 */

	/* 关闭I,DCache和MMU 
	 * 采取读-改-写的方式。
	 */
	mrc     p15, 0, r0, c1, c0, 0     /* 读取CP15的C1寄存器到R0中       		           */
    bic     r0,  r0, #(0x1 << 12)     /* 清除C1寄存器的bit12位(I位)，关闭I Cache            */
    bic     r0,  r0, #(0x1 <<  2)     /* 清除C1寄存器的bit2(C位)，关闭D Cache    			*/
    bic     r0,  r0, #0x2             /* 清除C1寄存器的bit1(A位)，关闭对齐					*/
    bic     r0,  r0, #(0x1 << 11)     /* 清除C1寄存器的bit11(Z位)，关闭分支预测				*/
    bic     r0,  r0, #0x1             /* 清除C1寄存器的bit0(M位)，关闭MMU				   */
    mcr     p15, 0, r0, c1, c0, 0     /* 将r0寄存器中的值写入到CP15的C1寄存器中	 		     */

	
#if 0
	/* 汇编版本设置中断向量表偏移 */
	ldr r0, =0X87800000

	dsb                               /*数据同步*/
	isb                               /*指令同步*/
	mcr p15, 0, r0, c12, c0, 0
	dsb
	isb
#endif
    
	/* 设置各个模式下的栈指针，
	 * 注意：IMX6UL的堆栈是向下增长的！
	 * 堆栈指针地址一定要是4字节地址对齐的！！！
	 * DDR范围:0X80000000~0X9FFFFFFF
	 */
	/* 进入IRQ模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	*/
	orr r0, r0, #0x12 	/* r0或上0x13,表示使用IRQ模式					 */
	msr cpsr, r0		/* 将r0 的数据写入到cpsr_c中 					 */
	ldr sp, =0x80600000	/* 设置IRQ模式下的栈首地址为0X80600000,大小为2MB   */

	/* 进入SYS模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	   */
	orr r0, r0, #0x1f 	/* r0或上0x13,表示使用SYS模式					 */
	msr cpsr, r0		/* 将r0 的数据写入到cpsr_c中 					 */
	ldr sp, =0x80400000	/* 设置SYS模式下的栈首地址为0X80400000,大小为2MB   */

	/* 进入SVC模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	*/
	orr r0, r0, #0x13 	/* r0或上0x13,表示使用SVC模式				  */
	msr cpsr, r0		    /* 将r0 的数据写入到cpsr_c中              */
	// cps     #0x13					
	ldr sp, =0X80200000	/* 设置SVC模式下的栈首地址为0X80200000,大小为2MB */


	// cps     #0x12                /* Enter IRQ mode                */
    // ldr     sp, =0x80600000      /* Set up IRQ handler stack      */

    // cps     #0x1F                /* Enter System mode             */
    // ldr     sp, =0x80400000      /* Set up System/User Mode stack */

    // cps     #0x13                /* Enter Supervisor mode         */
    // ldr     sp, =0x80200000      /* Set up Supervisor Mode stack  */

	ldr     r0,=SystemInit
    blx     r0

	cpsie i				/* 打开全局中断 */
#if 0
	/* 使能IRQ中断 */
	mrs r0, cpsr		/* 读取cpsr寄存器值到r0中 			 */
	bic r0, r0, #0x80	/* 将r0寄存器中bit7清零，也就是CPSR中的I位清零，表示允许IRQ中断 */
	msr cpsr, r0		/* 将r0重新写入到cpsr中 			*/
#endif

	b main				/* 跳转到main函数 			 	   */



/* 未定义中断 */
.align 2
.arm
.weak Undefined_Handler
.type Undefined_Handler, %function
Undefined_Handler:
	ldr r0, =Undefined_Handler
	bx r0

/* SVC中断 */
.align 2
.arm
.weak SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
    // b put
	ldr r0, =SVC_Handler
	bx r0

/* 预取终止中断 */
.align 2
.arm
.weak PrefAbort_Handler
.type PrefAbort_Handler, %function
PrefAbort_Handler:
	ldr r0, =PrefAbort_Handler	
	bx r0

/* 数据终止中断 */
.align 2
.arm
.weak DataAbort_Handler
.type DataAbort_Handler, %function
DataAbort_Handler:
	ldr r0, =DataAbort_Handler
	bx r0

/* 未使用的中断 */
.align 2
.arm
.weak NotUsed_Handler
.type NotUsed_Handler, %function
NotUsed_Handler:
	ldr r0, =NotUsed_Handler
	bx r0

/* IRQ中断！重点！！！！！ ，进入中断后，系统必然进入IRQ模式*/
.align 2
.arm
.weak IRQ_Handler
.type IRQ_Handler, %function
IRQ_Handler:
	push {lr}					/* 保存lr地址 */
	push {r0-r3, r12}			/* 保存r0-r3，r12寄存器 */

	mrs r0, spsr				/* 读取spsr寄存器 */
	push {r0}					/* 保存spsr寄存器 */

	mrc p15, 4, r1, c15, c0, 0  /* 从CP15的C0寄存器内的值到R1寄存器中
								* 参考文档ARM Cortex-A(armV7)编程手册V4.0.pdf P49
								* Cortex-A7 Technical ReferenceManua.pdf P68 P138
								*/							
	add r1, r1, #0X2000			/* GIC基地址加0X2000，也就是GIC的CPU接口端基地址 */
	ldr r0, [r1, #0XC]			/* GIC的CPU接口端基地址加0X0C就是GICC_IAR寄存器，
								 * GICC_IAR寄存器保存这当前发生中断的中断号，我们要根据
								 * 这个中断号来绝对调用哪个中断服务函数
								 */
	push {r0, r1}				/* 保存r0,r1 */
	
	cps #0x13					/* 进入SVC模式，允许其他中断再次进去 */
	
	push {lr}					/* 保存SVC模式的lr寄存器 */
	ldr r2, =SystemIrqHandler	/* 加载C语言中断处理函数到r2寄存器中，汇编传参，参数少于三个的时候，参数保存在r0*/
	blx r2						/* 运行C语言中断处理函数，带有一个参数，保存在R0寄存器中 */

	pop {lr}					/* 执行完C语言中断服务函数，lr出栈 */
	cps #0x12					/* 进入IRQ模式 */
	pop {r0, r1}				
	str r0, [r1, #0X10]			/* 中断执行完成，写EOIR */

	pop {r0}						
	msr spsr_cxsf, r0			/* 恢复spsr */

	pop {r0-r3, r12}			/* r0-r3,r12出栈 */
	pop {lr}					/* lr出栈 */
	subs pc, lr, #4				/* 将lr-4赋给pc */
	
	

/* FIQ中断 */
.align 2
.arm
.weak FIQ_Handler
.type FIQ_Handler, %function
FIQ_Handler:
	ldr r0, =FIQ_Handler	
	bx r0									












