/*
 *	PCB.H
 *	Process Control Block Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef PCB_H
#define PCB_H

#include "uARMtypes.h"
#include "types.h"
#include "clist.h"

/* Process List Management */

EXTERN void freePcb(pcb_t *p);
EXTERN pcb_t *allocPcb();
EXTERN void initPcbs();

EXTERN void insertProcQ(struct clist *q,  pcb_t *p);
EXTERN pcb_t *removeProcQ(struct clist *q);
EXTERN pcb_t *outProcQ(struct clist *q,  pcb_t *p);
EXTERN pcb_t *headProcQ(struct clist *q);

/* Process Tree Management */

EXTERN int emptyChild( pcb_t *p);
EXTERN void insertChild(pcb_t *parent, pcb_t *p);
EXTERN pcb_t *removeChild(pcb_t *p);
EXTERN pcb_t *outChild(pcb_t *p);

/* Auiliary Macros */

#define init_state_t(p) ({																			\
	p.a1 = p.a2 = p.a3 = p.a4 = 0;																	\
	p.v1 = p.v2 = p.v3 = p.v4 = p.v5 = p.v6 = 0;													\
	p.sl = p.fp = p.ip = p.sp = p.lr = p.pc = p.cpsr = 0;											\
	p.CP15_Control = p.CP15_EntryHi = p.CP15_Cause = p.TOD_Hi = p.TOD_Low = 0;						\
})

#endif
