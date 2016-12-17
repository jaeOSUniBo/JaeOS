/*
 *	CONST.H
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

/********************************
 * phase 2 constants definitions
 ********************************/

#ifndef CONST_H
#define CONST_H

#include <arch.h>

#define EXTERN extern
#define memaddr unsigned int

#ifndef TRUE
	#define TRUE 1
	#define FALSE 0
#endif

/* Maximum number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

/* Scheduling constants */
#define SCHED_TIME_SLICE 5000     /* in microseconds, aka 5 milliseconds */
#define SCHED_PSEUDO_CLOCK 100000 /* pseudo-clock tick "slice" length */
#define SCHED_BOGUS_SLICE 500000  /* just to make sure */

/* nucleus (phase2)-handled SYSCALL values */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define SEMOP 3
#define SPECSYSHDL 4
#define SPECTLBHDL 5
#define SPECPGMTHDL 6
#define EXITTRAP 7
#define GETCPUTIME 8
#define WAITCLOCK 9
#define IODEVOP 10
#define GETPID 11

#define SYSCALL_MAX 11

/* pcb exception states vector constants */
#define EXCP_SYS_OLD 0
#define EXCP_TLB_OLD 1
#define EXCP_PGMT_OLD 2
#define EXCP_SYS_NEW 3
#define EXCP_TLB_NEW 4
#define EXCP_PGMT_NEW 5

#define EXCP_COUNT 6

/* device types count with separate terminal read and write devs */
#define N_DEV_TYPES (N_EXT_IL+1)

#endif
