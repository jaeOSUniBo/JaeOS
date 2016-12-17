/*
 *	EXCEPTIONS.H
 *	TLB, PgmTrap and SYS/Bp Exception Handlers Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef EXCEPITION_H
#define EXCEPITION_H

#include <const.h>
#include <initial.h>

EXTERN cputime_t kernelStart;

// auxiliary prototype

EXTERN void saveCurState(state_t *state, state_t *newState);

// Function to handle common operations between different types of exception handlers
void handlerSYSTLBPGM(hdl_type old, UI new, state_t* state);

void tlbHandler();		// TLB Handler function
void sysHandler();		// SYSCALL/BP Handler function
void pgmHandler();		// Program Trap Handler function


/* Syscall prototypes (see syscall.h for details) */

EXTERN int createProcess(state_t *stato);
EXTERN void terminateProcess(pid_t p);
EXTERN void semaphoreOperation(int *sem, int weight);
EXTERN void specifySysBp(memaddr handler, memaddr stack, UI flags);
EXTERN void specifyTlb(memaddr handler, memaddr stack, UI flags);
EXTERN void specifyPgm(memaddr handler, memaddr stack, UI flags);
EXTERN void exitTrap(hdl_type exType, UI ret);
EXTERN void getCpuTime(cputime_t *global_time, cputime_t *user_time);
EXTERN void waitForClock();
EXTERN UI iodevop(UI command, int lineNum, UI deviceNum);
EXTERN pid_t getPid();

#endif
