
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

/* Dining philosophers problem implementation
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

#include "philosophers.h"

#define mainTIMER_DELAY (1000UL)

static void philosopherTask(void *pvParameters);
static void supervisorTask(void *pvParameters);

static struct Philosopher philosophers[philoPHILOSOPHERS_NUMBER];
static struct FSMState states[STATE_MAX];
static struct Chopstick chopsticks[philoPHILOSOPHERS_NUMBER] = {{0, 1},
								{1, 1},
								{2, 1},
								{3, 1},
								{4, 1}};

/*-----------------------------------------------------------*/

int main(void)
{
	TaskHandle_t clearTaskHandle;

	initPeripherals();

	xTaskCreate(supervisorTask, "supervisor", configMINIMAL_STACK_SIZE,
		    philosophers, philoCHECK_TASK_PRIORITY + 5,
		    &clearTaskHandle);

	initPhilosophers();

	vTaskStartScheduler();

	deinitPhilosophers();
	vTaskDelete(clearTaskHandle);

	return 0;
}

/*
 * Task print philosopher statistic every mainTIMER_DELAY miliseconds
 */
static void supervisorTask(void *pvParameters)
{
	const TickType_t xCycleFrequency = pdMS_TO_TICKS(mainTIMER_DELAY);
	TickType_t xNextWakeTime = xTaskGetTickCount();
	struct Philosopher *xPhilosophers = (struct Philosopher *)pvParameters;

	for (;;) {
		uint i;

		vTaskDelayUntil(&xNextWakeTime, xCycleFrequency);
		clearScreen();
		logMessage("\n\r");

		taskENTER_CRITICAL();
		for (i = 0; i < philoPHILOSOPHERS_NUMBER; ++i) {
			logMessage("philosopher %u ate %u in %u \n\r",
			       xPhilosophers[i].id, xPhilosophers[i].ate,
			       xPhilosophers[i].cycles);
		}

		logMessage("\n\r");
		for (i = 0; i < philoPHILOSOPHERS_NUMBER; ++i) {
			if (xPhilosophers[i].hasRightChopstick) {
				logMessage("philosopher %u has right chopstick\n\r",
				       xPhilosophers[i].id);
			} else if (checkRightChopstick(&xPhilosophers[i])) {
				logMessage("philosopher %u right is available\r\n",
				       xPhilosophers[i].id);
			}

			if (xPhilosophers[i].hasLeftChopstick) {
				logMessage("philosopher %u has left chopstick\n\r",
				       xPhilosophers[i].id);
			} else if (checkLeftChopstick(&xPhilosophers[i])) {
				logMessage("philosopher %u left is available\r\n",
				       xPhilosophers[i].id);
			}
		}
		taskEXIT_CRITICAL();
	}
}

/*
 * Philosopher FSM entry point
 */
static void philosopherTask(void *pvParameters)
{
	struct Philosopher *xPhilosopher = (struct Philosopher *)pvParameters;
	for (;;) {
		FSMNextStep(xPhilosopher, states);
	}
	vTaskSuspend(NULL);
}

/*
 * Creates tasks that implements philosophers and initialize FSM states.
 */
void initPhilosophers(void)
{
	uint i = 0;

	for (i = 0; i < philoPHILOSOPHERS_NUMBER; ++i) {
		philosophers[i].id = i;
		philosophers[i].state = THINKING;
		philosophers[i].chopsticks = chopsticks;
		xTaskCreate(philosopherTask, "philosopherTask", configMINIMAL_STACK_SIZE,
			    &philosophers[i], philoCHECK_TASK_PRIORITY,
			    &(philosophers[i].taskHandle));
	}

	FSMInit(states, philosophers);
}

void deinitPhilosophers(void)
{
	uint i = 0;

	for (i = 0; i < philoPHILOSOPHERS_NUMBER; ++i) {
		vTaskDelete(philosophers[i].taskHandle);
	}
}

/*-----------------------------------------------------------*/

void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	taskENTER_CRITICAL();
	{
		printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
		fflush(stdout);
	}
	taskEXIT_CRITICAL();
	exit(-1);
}
/*-----------------------------------------------------------*/
