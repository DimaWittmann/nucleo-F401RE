#include <stdio.h>
#include <stdarg.h>

#include "FreeRTOS.h"

#include "main.h"
#include "philosophers.h"

static uint getRightChopstickId(struct Philosopher *p) {
	uint id = 0;
	if (p->id > 0) {
		id = (int) p->id - 1;
	} else {
		id = philoPHILOSOPHERS_NUMBER - 1;
	}

	return id;
}

void FSMNextStep(struct Philosopher *p, struct FSMState states[STATE_MAX]) {
	p->state = states[p->state].action(p);
	p->cycles++;
}

bool checkLeftChopstick(struct Philosopher *p) {
	return p->chopsticks[p->id].available;
}

bool checkRightChopstick(struct Philosopher *p) {
	return p->chopsticks[getRightChopstickId(p)].available;
}

void getLeftChopstick(struct Philosopher *p) {
	p->chopsticks[p->id].available = 0;
	p->hasLeftChopstick = true;

	logDebugMessage("philosopher %u got %u chopstick \n\r", p->id, p->id);
}

void getRightChopstick(struct Philosopher *p) {
	const uint chopstickId = getRightChopstickId(p);

	p->chopsticks[chopstickId].available = 0;
	p->hasRightChopstick = true;

	logDebugMessage("philosopher %u got %u chopstick \n\r", p->id, chopstickId);
}

void putLeftChopstick(struct Philosopher *p) {
	p->chopsticks[p->id].available = 1;
	p->hasLeftChopstick = false;

	logDebugMessage("philosopher %u put %u chopstick \n\r", p->id, p->id);
}

void putRightChopstick(struct Philosopher *p) {
	const uint chopstickId = getRightChopstickId(p);
	p->chopsticks[chopstickId].available = 1;
	p->hasRightChopstick = false;

	logDebugMessage("philosopher %u put %u chopstick \n\r", p->id, chopstickId);
}

const char *stateToChar(enum State state) {
	const char *str = "";

	switch (state) {
	case EATING:
		str = "EATING";
		break;
	case THINKING:
		str = "THINKING";
		break;
	case HUNGRY:
		str = "HUNGRY";
		break;
	default:
		configASSERT(0)
		;
	}

	return str;
}

