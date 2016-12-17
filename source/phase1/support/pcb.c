/*
 *	PCB.C
 *	Process Control Block Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include "const.h"
#include "clist.h"
#include "pcb.h"

void initPcbType(pcb_t *pcb);

HIDDEN struct clist pcbFree;

/***************************************************************
*           ALLOCATION AND DEALLOCATION PROCKBLKS'S            *
***************************************************************/

void freePcb(struct pcb_t *p) {
	// If the ProcBlk pointed to by p is not NULL, it gets deallocated
	// by adding it to the pcbFree list. Fields don't get changed because
	// they will be set next time the ProcBlk will be allocated and reused.
	if (p != NULL) clist_enqueue(p, &pcbFree, p_list);
}

struct pcb_t* allocPcb() {
	struct pcb_t *head = NULL;

	// If the pcbFree list is not empty
	// a new ProcBlk can be allocated
	if (!clist_empty(pcbFree)) {
		// Getting a pointer to the first free ProcBlk
		head = clist_head(head, pcbFree, p_list);
		clist_dequeue(&pcbFree);	// and removing it from the pcbFree list

		// Initializing ProcBlk fields
		initPcbType(head);
	}

	// Returning pointer to the allocated ProcBlk
	return head;
}

void initPcbs(void) {
	// Initialization
	pcbFree = (struct clist)CLIST_INIT;
	static struct pcb_t procArray[MAXPROC];

	// Adding every ProcBlk to the pcbFree list
	int i;
	for (i = 0; i < MAXPROC; i++) {
		struct pcb_t *elem = &(procArray[i]);
		clist_enqueue(elem, &pcbFree, p_list);
	}
}

/***************************************************************
*                   PROCESS QUEUE MAINTENANCE                  *
***************************************************************/

void insertProcQ(struct clist *q, struct pcb_t *p) {
	// If the ProcBlk pointed to by p is not NULL, it will be added to
	// the process queue whose tail pointer is q. If the process queue
	// pointed to by q is NULL, it will be set up when adding the
	// first ProcBlk.
	if (p != NULL) clist_enqueue(p, q, p_list);
}

struct pcb_t* headProcQ(struct clist *q) {
	// Getting a pointer to the first ProcBlk in the process queue
	// whose tail pointer is q and returning it without removing it
	struct pcb_t *first = clist_head(first, (*q), p_list);
	return first;
}

struct pcb_t* removeProcQ(struct clist *q) {
	if (q->next == NULL) return NULL;

	// Getting a pointer to the first ProcBlk in the process queue
	// whose tail pointer is q and returning it after removing it
	// from the process queue
	struct pcb_t *head = clist_head(head, (*q), p_list);
	clist_dequeue(q);
	return head;
}

struct pcb_t* outProcQ(struct clist *q, struct pcb_t *p) {
	if (q->next == NULL || p == NULL) return NULL;

	// Searching the ProcBlk pointed to by p in the process queue
	// whose tail pointer is q, removing it from the process queue
	// and returning it.
	if (clist_delete(p, q, p_list) != 0) return NULL;
	else return p;
}

/***************************************************************
*                   PROCESS TREE MAINTENANCE                   *
***************************************************************/

int emptyChild(struct pcb_t *p) {
	if (p != NULL && clist_empty(p->p_children)) return TRUE;
	else return FALSE;
}

void insertChild(struct pcb_t *parent, struct pcb_t *p) {
	if (parent != NULL && p != NULL) {
		p->p_parent = parent;	// Settings parent pointer of p

		// Adding the ProcBlk pointed to by p to the list of
		// children of the ProcBlk pointed to by parent
		clist_enqueue(p, &(parent->p_children), p_siblings);
	}
}

struct pcb_t* removeChild(struct pcb_t *p) {
	if (p == NULL || emptyChild(p)) return NULL;

	// Getting a pointer to the first child of ProcBlk pointed
	// to by p, removing it from the list of children and returning it
	struct pcb_t *head = clist_head(head, p->p_children, p_siblings);
	clist_dequeue(&(p->p_children));
	head->p_parent = NULL;
	return head;
}

struct pcb_t* outChild(struct pcb_t *p) {
	if (p == NULL || p->p_parent == NULL) return NULL;

	// Getting the tail pointer to the list of children
	struct clist *children = &(p->p_parent->p_children);

	// Searching the ProcBlk pointed to by p, removing it from
	// the list of children and returning it
	if (clist_delete(p, children, p_siblings) != 0) return NULL;
	else {
		p->p_parent = NULL;
		return p;
	}
}

/***************************************************************
*                      AUXILIARY FUNCTION                      *
***************************************************************/

void initPcbType(pcb_t *pcb) {
	int i;

	pcb->p_list = (struct clist)CLIST_INIT;
	pcb->p_children = (struct clist)CLIST_INIT;
	pcb->p_siblings = (struct clist)CLIST_INIT;
	pcb->p_parent = NULL;
	pcb->p_cursem = NULL;

	init_state_t(pcb->p_s);

	for(i=0; i<EXCP_COUNT;i++)
		init_state_t(pcb->excp_state_vector[i]);

	pcb->pid = (int)pcb;
	pcb->kernel_mode = 0;
	pcb->global_time = 0;
	pcb->remaining = 5000;
	pcb->waitingResCount = 0;
	pcb->tags = 0;
}
