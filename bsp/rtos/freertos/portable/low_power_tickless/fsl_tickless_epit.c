/*
	FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
	All rights reserved

	VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License (version 2) as published by the
	Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

	***************************************************************************
	>>!   NOTE: The modification to the GPL is included to allow you to     !<<
	>>!   distribute a combined work that includes FreeRTOS without being   !<<
	>>!   obliged to provide the source code for proprietary components     !<<
	>>!   outside of the FreeRTOS kernel.                                   !<<
	***************************************************************************

	FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE.  Full license text is available on the following
	link: http://www.freertos.org/a00114.html

	***************************************************************************
	 *                                                                       *
	 *    FreeRTOS provides completely free yet professionally developed,    *
	 *    robust, strictly quality controlled, supported, and cross          *
	 *    platform software that is more than just the market leader, it     *
	 *    is the industry's de facto standard.                               *
	 *                                                                       *
	 *    Help yourself get started quickly while simultaneously helping     *
	 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
	 *    tutorial book, reference manual, or both:                          *
	 *    http://www.FreeRTOS.org/Documentation                              *
	 *                                                                       *
	***************************************************************************

	http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
	the FAQ page "My application does not run, what could be wrong?".  Have you
	defined configASSERT()?

	http://www.FreeRTOS.org/support - In return for receiving this top quality
	embedded software for free we request you assist our global community by
	participating in the support forum.

	http://www.FreeRTOS.org/training - Investing in training allows your team to
	be as productive as possible as early as possible.  Now you can receive
	FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
	Ltd, and the world's leading authority on the world's leading RTOS.

	http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool, a DOS
	compatible FAT file system, and our tiny thread aware UDP/IP stack.

	http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
	Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

	http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
	Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
	licenses offer ticketed support, indemnification and commercial middleware.

	http://www.SafeRTOS.com - High Integrity Systems also provide a safety
	engineered and independently SIL3 certified version for use in safety and
	mission critical applications that require provable dependability.

	1 tab == 4 spaces!
*/
/* Compiler includes. */
#if defined(__ICCARM__)
#include <intrinsics.h>
#endif

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_tickless_generic.h"
#include "bsp_clk.h"

#if configUSE_TICKLESS_IDLE == 1
#include "fsl_epit.h"
#endif

extern uint32_t SystemCoreClock; /* in Kinetis SDK, this contains the system core clock speed */

/*
 * EPIT timer base address and interrupt number
 */

#if configUSE_TICKLESS_IDLE == 1
	extern EPIT_Type *vPortGetTimerBase(void);
	extern IRQn_Type vPortGetTimerIrqn(void);
#endif /* configUSE_TICKLESS_IDLE */

/*-----------------------------------------------------------*/

/*
 * The number of SysTick increments that make up one tick period.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t ulTimerCountsForOneTick = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The number of EPIT increments that make up one tick period.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t ulEPITimerCountsForOneTick = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The flag of EPIT is occurs or not.
 */
#if configUSE_TICKLESS_IDLE == 1
	static volatile bool ulEPITimerInterruptFired = false;
#endif /* configUSE_TICKLESS_IDLE */

#if configUSE_TICKLESS_IDLE == 1
	void vPortTimerIsr(void)
	{
		ulEPITimerInterruptFired = true;
		EPIT_ClearStatusFlags(vPortGetTimerBase(), kEPIT_OutputCompareFlag);
	}

	void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
	{
		uint32_t ulReloadValue, ulCompleteTickPeriods;
		TickType_t xModifiableIdleTime;
		EPIT_Type *pxEpitBase;
	
		if (ulTimerCountsForOneTick == 0)
		{
			ulTimerCountsForOneTick = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);
			ulStoppedTimerCompensation = 0;
			ulEPITimerCountsForOneTick = configEPIT_RATE_HZ / configTICK_RATE_HZ;
			xMaximumPossibleSuppressedTicks = portMAX_32_BIT_NUMBER / ulEPITimerCountsForOneTick;
		}
	
		pxEpitBase = vPortGetTimerBase();
		if (pxEpitBase == 0)
			return;
		/* Make sure the SysTick reload value does not overflow the counter. */
		if (xExpectedIdleTime > xMaximumPossibleSuppressedTicks)
		{
			xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
		}
		if (xExpectedIdleTime == 0)
			return;
	
		/* Calculate the reload value required to wait xExpectedIdleTime
		tick periods.  -1 is used because this code will execute part way
		through one of the tick periods. */
		ulReloadValue = (ulEPITimerCountsForOneTick * (xExpectedIdleTime - 1UL));
		if (ulReloadValue > ulStoppedTimerCompensation)
		{
			ulReloadValue -= ulStoppedTimerCompensation;
		}
	
		/* Stop the EPIT and systick momentarily.  The time the EPIT and systick is stopped for
		is accounted for as best it can be, but using the tickless mode will
		inevitably result in some tiny drift of the time maintained by the
		kernel with respect to calendar time. */
		EPIT_StopTimer(pxEpitBase);
		portGIC_SYSTICK_CTRL_REG &= ~portGIC_SYSTICK_ENABLE_BIT;
	
		/* Enter a critical section but don't use the taskENTER_CRITICAL()
		method as that will mask interrupts that should exit sleep mode. */
		__disable_irq();
		__DSB();
		__ISB();
	
		/* If a context switch is pending or a task is waiting for the scheduler
		to be unsuspended then abandon the low power entry. */
		if (eTaskConfirmSleepModeStatus() == eAbortSleep)
		{
			/* Restart from whatever is left in the count register to complete
			this tick period. */
			portGIC_SYSTICK_LOAD_REG = portGIC_SYSTICK_CURRENT_VALUE_REG;
	
			/* Restart SysTick. */
			portGIC_SYSTICK_CTRL_REG |= portGIC_SYSTICK_ENABLE_BIT;
	
			/* Reset the reload register to the value required for normal tick
			periods. */
			portGIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
	
			/* Re-enable interrupts - see comments above __disable_irq()
			call above. */
			__enable_irq();
		}
		else
		{
			/* Set the new reload value. */
			EPIT_SetTimerPeriod(pxEpitBase, ulReloadValue);
			EPIT_SetOutputCompareValue(pxEpitBase, 0);
	
			/* Enable EPIT. */
			EPIT_StartTimer(pxEpitBase);
	
			/* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
			set its parameter to 0 to indicate that its implementation contains
			its own wait for interrupt or wait for event instruction, and so wfi
			should not be executed again.  However, the original expected idle
			time variable must remain unmodified, so a copy is taken. */
			xModifiableIdleTime = xExpectedIdleTime;
			configPRE_SLEEP_PROCESSING(xModifiableIdleTime);
			if (xModifiableIdleTime > 0)
			{
				__DSB();
				__WFI();
				__ISB();
			}
			configPOST_SLEEP_PROCESSING(xExpectedIdleTime);
	
			ulEPITimerInterruptFired = false;
	
			/* Re-enable interrupts - see comments above __disable_irq()
			call above. */
			__enable_irq();
			__NOP();
			if (ulEPITimerInterruptFired)
			{
				/* The tick interrupt handler will already have pended the tick
				processing in the kernel.  As the pending tick will be
				processed as soon as this function exits, the tick value
				maintained by the tick is stepped forward by one less than the
				time spent waiting. */
				ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
				ulEPITimerInterruptFired = false;
			}
			else
			{
				/* Something other than the tick interrupt ended the sleep.
				Work out how long the sleep lasted rounded to complete tick
				periods (not the ulReload value which accounted for part
				ticks). */
				ulCompleteTickPeriods =
					(ulReloadValue - EPIT_GetCurrentTimerCount(pxEpitBase)) / ulEPITimerCountsForOneTick;
			}
	
			/* Stop EPIT when CPU waked up then set portGIC_SYSTICK_LOAD_REG back to its standard
			value.  The critical section is used to ensure the tick interrupt
			can only execute once in the case that the reload register is near
			zero. */
			EPIT_StopTimer(pxEpitBase);
			portENTER_CRITICAL();
			{
				portGIC_SYSTICK_CTRL_REG |= portGIC_SYSTICK_ENABLE_BIT;
				vTaskStepTick(ulCompleteTickPeriods);
				portGIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
			}
			portEXIT_CRITICAL();
		}
	}
#endif /* #if configUSE_TICKLESS_IDLE */
