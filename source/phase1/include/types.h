/*
 *	TYPES.H
 *	Shared Types Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef TYPES_H
#define TYPES_H

#include "clist.h"
#include <uARMtypes.h>
#include "const.h"

typedef unsigned int size_t;
typedef unsigned int pid_t;
typedef unsigned int cputime_t;

typedef unsigned int UI;
typedef signed int SI;
typedef unsigned char UC;
typedef signed char SC;

struct semd_t;

/* process control block type */
typedef struct pcb_t {
	struct pcb_t *p_parent; 				/* pointer to parent */
	struct semd_t *p_cursem; 				/* pointer to the semd_t on which process blocked */
	pid_t pid; 								/* process id */
	state_t p_s; 							/* processor state */
	state_t excp_state_vector[EXCP_COUNT];
	struct clist p_list; 					/* process list */
	struct clist p_children; 				/* children list entry point*/
	struct clist p_siblings; 				/* children list: links to the siblings */
	cputime_t kernel_mode;
	cputime_t global_time;
	cputime_t remaining;
	UI waitingResCount;
	UI tags;
} pcb_t;

/* custom type to describe interrupt acknowledgements */
typedef enum ack_type {
	ACK_GEN_DEVICE,
	ACK_TERM_TRANSMIT,
	ACK_TERM_RECEIVE
} ack_type;

/* custom type to specify handler type */
typedef enum hdl_type {
	SYS_HDL,
	TLB_HDL,
	PGMT_HDL
} hdl_type;

#endif
