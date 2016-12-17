/*
 *	ASL.H
 *	Active Semaphore List Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef ASL_H
#define ASL_H

/* Sempahore Functions Prototypes */

EXTERN void initASL();
EXTERN int insertBlocked(int *semAdd, pcb_t *p); 	/* Insert ProcBlk pointed to by p to the semaphore */
EXTERN pcb_t *removeBlocked(int *semAdd); 			/* Remove first blocked ProcBlk on semAdd, if present */
EXTERN pcb_t *outBlocked(pcb_t *p); 				/* Remove ProcBlk pointed to by p */
EXTERN pcb_t *headBlocked(int *semAdd); 			/* Return a pointer to the first blocked ProcBlk to semAdd sempahore */

/* Auxiliary Functions */

EXTERN void updateSemaphoreValue(struct semd_t *sem, int delta);	/* Use negative delta values to decrement */

#endif

