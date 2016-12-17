/*
 *	ASL.C
 *	Active Semaphore List Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include "const.h"
#include "clist.h"
#include "pcb.h"
#include "asl.h"

/* semd_t Sempahore Structure */

typedef struct semd_t {
	int *s_semAdd;
	struct clist s_link;
	struct clist s_procq;
} semd_t;

HIDDEN struct clist aslh, semdFree;

/***************************************************************
*                    ACTIVE SEMAPHORE LIST                     *
***************************************************************/

void initASL() {
	// Initialization
	aslh = (struct clist)CLIST_INIT;
	semdFree = (struct clist)CLIST_INIT;
	static struct semd_t semdTable[MAXPROC];

	// Adding every sempahore to the semdFree list
	int i;
	for (i = 0; i < MAXPROC; i++) {
		struct semd_t *elem = &(semdTable[i]);
		clist_enqueue(elem, &semdFree, s_link);
	}
}

int insertBlocked(int *semAdd, struct pcb_t *p) {
	struct semd_t *scan = NULL;
	void *tmp = NULL;

	// Searching active sempahore whose identifier is semAdd
	// This cycle won't be executed when the aslh list if empty
	clist_foreach(scan, &aslh, s_link, tmp) {
		if (semAdd == scan->s_semAdd) break;
	}

	int res = clist_foreach_all(scan, &aslh, s_link, tmp);

	// The searched active semaphore doesn't exist (it may not be active)
	if (res != 0) {
		// If the semdFree list is not empty, there are free descriptors
		// and a new semaphore can be allocated
		if (!clist_empty(semdFree)) {
			// Getting a pointer to the first free descriptor and making
			// it active by dequeuing it from the semdFree list
			struct semd_t *head = NULL;
			head = clist_head(head, semdFree, s_link);
			clist_dequeue(&semdFree);

			// Find the correct position where to insert the new semaphore
			// in the aslh list to keep the ascending order, based on semAdd
			// This cycle won't be executed when the aslh list is empty
			clist_foreach(scan, &aslh, s_link, tmp) {
				if (semAdd < scan->s_semAdd) {
					clist_foreach_add(head, scan, &aslh, s_link, tmp);
					break;
				}
			}
			if (clist_foreach_all(scan, &aslh, s_link, tmp))
				clist_enqueue(head, &aslh, s_link);

			// Setting pointers to initialize semaphore fields
			scan = head;
			head = NULL;

			scan->s_semAdd = semAdd;
			scan->s_procq = (struct clist)CLIST_INIT;
		}
		// A new semaphore needs to be allocated but no free descriptors are available
		else return TRUE;
	}

	// Setting the blocked ProcBlk to the semaphore and adding it to the
	// process queue of the semaphore
	p->p_cursem = scan;
	insertProcQ(&(scan->s_procq), p);

	return FALSE;
}

struct pcb_t* removeBlocked(int *semAdd) {
	struct semd_t *scan = NULL;
	void *tmp = NULL;

	// Searching active sempahore whose identifier is semAdd
	// This cycle won't be executed when the aslh list if empty
	clist_foreach(scan, &aslh, s_link, tmp) {
		if (semAdd == scan->s_semAdd) break;
	}

	int res = clist_foreach_all(scan, &aslh, s_link, tmp);

	// Return NULL if the searched semaphore is not active (it may not be active)
	if (res != 0) return NULL;

	// Remove the first blocked ProcBlk from the process queue of the sempahore
	// whose identifier is semAdd and get a pointer to that ProcBlk
	struct pcb_t *proc = removeProcQ(&(scan->s_procq));
	proc->p_cursem = NULL;
	proc->p_list = (struct clist)CLIST_INIT;

	// Checking if the process queue of the semaphore gets empty after removing
	// a ProcBlk so that the entire sempahore can be deallocate and returned to
	// the semdFree list
	if (clist_empty(scan->s_procq)) {
		if (clist_delete(scan, &aslh, s_link) == 0) {
			clist_enqueue(scan, &semdFree, s_link);
		}
	}

	return proc;
}

struct pcb_t* outBlocked(struct pcb_t *p) {
	if (p->p_cursem == NULL) return NULL;

	// Getting a pointer to the process queue of the semaphore where
	// the ProcBlk pointed to by p is blocked against
	struct clist *procscan = &(p->p_cursem->s_procq);

	// Remove the ProcBlk from the process queue of the semphore
	struct pcb_t *proc = outProcQ(procscan, p);

	// Dealloc semaphore description if last blocked ProcBlk is removed
	if (clist_empty(p->p_cursem->s_procq)) {
		clist_delete(p->p_cursem, &aslh, s_link);
		clist_enqueue(p->p_cursem, &semdFree, s_link);
	}

	return proc;
}

struct pcb_t* headBlocked(int *semAdd) {
	struct semd_t *scan = NULL;
	void *tmp = NULL;

	// Searching active sempahore whose identifier is semAdd
	// This cycle won't be executed when the aslh list if empty
	clist_foreach(scan, &aslh, s_link, tmp) {
		if (semAdd == scan->s_semAdd) break;
	}

	int res = clist_foreach_all(scan, &aslh, s_link, tmp);

	// Return NULL if the searched semaphore is not found (it may not be active)
	if (res != 0) return NULL;
	else {
		// Return NULL is the semaphore is active but it has not processes
		// in its process queue
		if (clist_empty(scan->s_procq)) return NULL;

		// Getting a pointer to the first blocked ProcBlk of the semaphore
		struct pcb_t *head = headProcQ(&(scan->s_procq));
		return head;
	}
}

/***************************************************************
*                      AUXILIARY FUNCTION                      *
***************************************************************/

void updateSemaphoreValue(struct semd_t *sem, int delta) {
	if (!sem) PANIC();

	*(sem->s_semAdd) += delta;
}
