/*
 *	SCHEDULER.C
 *	Scheduling Policy and Deadlock Detection Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include <scheduler.h>

void scheduler() {

	// There is no executing process
	if (!curProc) {
		if (clist_empty(readyQueue)) {
			/* no more processes*/
			if (processCounter == 0) HALT();
			/* Deadlock Detection*/
			if (processCounter > 0 && softBlockCounter == 0) PANIC();
			/* wait for interrupts */
			if (processCounter > 0 && softBlockCounter > 0) {
				setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
				WAIT();
			}
			PANIC();
		}

		// Loading first process in ready queue
		if (!(curProc = removeProcQ(&readyQueue))) PANIC();

		// Updating pseudo clock and new time interval
		clock += getTODLO() - clockTick;
		clockTick = getTODLO();
		processStart = getTODLO();	// Starting process execution interval

		// Setting timer to remaining process time slice (to avoid delays)
		if (curProc->remaining < SCHED_TIME_SLICE) setTIMER(curProc->remaining);
		else setTIMER(SCHED_TIME_SLICE);
	}
	// There is a running process
	else {
		curProc->global_time += getTODLO() - processStart;		// updating process global time

		clock += getTODLO() - clockTick;
		clockTick = getTODLO();

		// Setting timer to remaining time slice
		setTIMER(SCHED_TIME_SLICE - (getTODLO() - processStart));
		processStart = getTODLO();
	}

	LDST(&(curProc->p_s));
}
