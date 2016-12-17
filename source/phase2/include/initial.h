/*
 *	INITIAL.H
 *	Kernel Entry Point Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef INITIAL_H
#define INITIAL_H

#include <uARMconst.h>
#include <uARMtypes.h>
#include <arch.h>
#include <pcb.h>
#include <asl.h>
#include <exceptions.h>
#include <interrupts.h>
#include <libuarm.h>

/* Data Structures used for Process/Kernel execution */

UI processCounter;					// number of total processes
UI softBlockCounter;				// number of processes waiting for an interrupt
EXTERN struct clist readyQueue;		// ready queue structure
int semDevices[MAX_DEVICES];		// array of device semaphores
pcb_t *curProc;						// current process executing


/* Data Structurer for Time Management */

cputime_t kernelStart;				// variable to keep track of kernel execution start
cputime_t clock;					// pseudo clock total value
cputime_t clockTick;				// time interval between events to be added to the pseudo clock
cputime_t processStart;				// variable to keep track of process execution start or resume


// Auxiliary function used to initialize uARM default handler areas
void initArea(memaddr area, memaddr handler);


// extern modules function

EXTERN void test();
EXTERN void scheduler();
EXTERN pid_t genPid(UI a);

#endif
