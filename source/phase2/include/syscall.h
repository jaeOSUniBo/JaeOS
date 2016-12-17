/*
 *	SYSCALL.H
 *	System Calls Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <initial.h>

/* Auxiliary Functions */

// Function to copy STATE fields to NEWSTATE fields
void saveCurState(state_t *state, state_t *newState);
// Pseudo random PID generator
pid_t genPid(UI a);
// Recursive function used to search ProcBlk in PARENT hierarchy, using its PID 
pcb_t* searchPid(pcb_t *parent, pid_t pid);
// Recursive function used to kill and free PROC and all its progeny
void terminator(pcb_t* proc);
// Function used to manage common operation when setting exception handlers
void setSYSTLBPGMT(hdl_type old, UI new, memaddr handler, memaddr stack, UI flags);

/* System Calls */

int createProcess(state_t *stato);									// SYS 1
void terminateProcess(pid_t p);										// SYS 2
void semaphoreOperation(int *sem, int weight);						// SYS 3
void specifySysBp(memaddr handler, memaddr stack, UI flags);		// SYS 4
void specifyTlb(memaddr handler, memaddr stack, UI flags);			// SYS 5
void specifyPgm(memaddr handler, memaddr stack, UI flags);			// SYS 6
void exitTrap(hdl_type exType, UI ret);								// SYS 7
void getCpuTime(cputime_t *global_time, cputime_t *user_time);		// SYS 8
void waitForClock();												// SYS 9
UI iodevop(UI command, int lineNum, UI deviceNum);					// SYS 10
pid_t getPid();														// SYS 11

#endif
