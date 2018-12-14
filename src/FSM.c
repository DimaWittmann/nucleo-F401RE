#include <stdio.h>
#include <limits.h>

#include "FreeRTOS.h"

#include "philosophers.h"

#define philoWAITING_DELAY (100UL)

void FSMInit(struct FSMState states[STATE_MAX],
		 struct Philosopher philosophers[philoPHILOSOPHERS_NUMBER])
{
	states[NOOP].action = FSMNoOp;
	states[THINKING].action = FSMThinking;
	states[EATING].action = FSMEating;
	states[HUNGRY].action = FSMHungry;

	(void) philosophers;
}


enum State FSMNoOp(struct Philosopher *p)
{
	(void) p;
	return NOOP;
}


enum State FSMThinking(struct Philosopher *philosopher)
{
	(void) philosopher;
	return HUNGRY;
}


enum State FSMHungry(struct Philosopher *philosopher)
{
	if (philosopher->hasLeftChopstick && philosopher->hasRightChopstick) {
		return EATING;
	}

	return HUNGRY;
}


enum State FSMEating(struct Philosopher *philosopher)
{
	(void) philosopher;
	return THINKING;
}
