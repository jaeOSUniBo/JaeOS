/*
 *	EXCEPTIONS.C
 *	TLB, PgmTrap and SYS/Bp Exception Handlers Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include <exceptions.h>

// Pointers initialization to uARM memory area locations
state_t *tlb_old = (state_t*)TLB_OLDAREA;
state_t *pgmtrap_old = (state_t*)PGMTRAP_OLDAREA;
state_t *sysbp_old = (state_t*)SYSBK_OLDAREA;


/***************************************************************
*                      AUXILIARY FUNCTION                      *
***************************************************************/

void handlerSYSTLBPGM(hdl_type old, UI new, state_t* state) {
	if (old == TLB_HDL) {
		// if TLB Handler hasn't been set, act as SYS 2
		if (curProc->tags == 0 || curProc->tags == 1 || curProc->tags == 4 || curProc->tags == 5) {
			terminateProcess(0);
			scheduler();
		}
	}
	else if (old == PGMT_HDL) {
		// if Program Trap Handler hasn't been set, act as SYS 2
		if (curProc->tags == 0 || curProc->tags == 1 || curProc->tags == 2 || curProc->tags == 3) {
			terminateProcess(0);
			scheduler();
		}
	}
	else if (old == SYS_HDL) {
		UI a1 = state->a1;
		UI a2 = state->a2;
		UI a3 = state->a3;
		UI a4 = state->a4;

		// Invoke the appropriate SYSCALL by analyzing its value contiained in A1 register
		if (state->CP15_Cause == EXC_SYSCALL) {
			switch (a1) {
				case CREATEPROCESS:
					curProc->p_s.a1 = createProcess((state_t *)a2);
					break;
				case TERMINATEPROCESS:
					terminateProcess(a2);
					break;
				case SEMOP:
					semaphoreOperation((int *)a2, a3);
					break;
				case SPECSYSHDL:
					specifySysBp(a2, a3, a4);
					break;
				case SPECTLBHDL:
					specifyTlb(a2, a3, a4);
					break;
				case SPECPGMTHDL:
					specifyPgm(a2, a3, a4);
					break;
				case EXITTRAP:
					exitTrap(a2, a3);
					break;
				case GETCPUTIME:
					getCpuTime((cputime_t *)a2, (cputime_t *)a3);
					kernelStart = getTODLO();
					break;
				case WAITCLOCK:
					waitForClock();
					break;
				case IODEVOP:
					curProc->p_s.a1 = iodevop(a2, a3, a4);
					break;
				case GETPID:
					curProc->p_s.a1 = getPid();
					break;
				default:
					PANIC();
			}
		}
	}
	else PANIC();

	if (old != SYS_HDL) {
		// Save current process state as old one so that when returning from exception handling
		// the process can continue its execution from where it left
		saveCurState(state, &(curProc->excp_state_vector[old]));						
		curProc->excp_state_vector[new].a1 = CAUSE_EXCCODE_GET(state->CP15_Cause);
		curProc->kernel_mode += getTODLO() - kernelStart; 		// updating process kernel time
		LDST(&(curProc->excp_state_vector[new]));
	}
}

/***************************************************************
*                           HANDLERS                           *
***************************************************************/

void tlbHandler() {
	kernelStart = getTODLO();								// Start kernel time interval
	handlerSYSTLBPGM(TLB_HDL, EXCP_TLB_NEW, tlb_old);		// Handle TLB exception
}

void sysHandler() {
	kernelStart = getTODLO();		// Start kernel time interval
	
	// Checking if a reserved SYSCALL has been requested (SYS 1 - SYS 11)
	if (CAUSE_EXCCODE_GET(sysbp_old->CP15_Cause) == EXC_SYSCALL && sysbp_old->a1 >= 1 && sysbp_old->a1 <= 11) {
		// if CPU is in kernel mode, SYSCALL can be invoked
		if ((curProc->p_s.cpsr & STATUS_SYS_MODE) == STATUS_SYS_MODE) {
			saveCurState(sysbp_old, &curProc->p_s);
			handlerSYSTLBPGM(SYS_HDL, EXCP_SYS_NEW, sysbp_old);		// Handle SYSCALL exception
			curProc->kernel_mode += getTODLO() - kernelStart;		// updating process kernel time
			scheduler();
		}
		// if CPU is in user mode, a Program Trap must be issued to handle a reserved SYSCALL
		else if ((curProc->p_s.cpsr & STATUS_USER_MODE) == STATUS_USER_MODE) {
			saveCurState(sysbp_old, pgmtrap_old);
			pgmtrap_old->CP15_Cause = CAUSE_EXCCODE_SET(pgmtrap_old->CP15_Cause, EXC_RESERVEDINSTR);		// Setting RESERVED INSTRUCTION as cause
			curProc->kernel_mode += getTODLO() - kernelStart;	// updating process kernel time
			pgmHandler();										// Handle Program Trap
		}
	}
	// Any other SYSCALL or BP has been requested
	else {
		// if SYSCALL handler hasn't been set, act as SYS 2
		if (curProc->tags != 1 && curProc->tags != 3 && curProc->tags != 5 && curProc->tags != 7) {
			terminateProcess(curProc->pid);
			scheduler();
		}

		saveCurState(sysbp_old, &(curProc->excp_state_vector[EXCP_SYS_OLD]));

		curProc->excp_state_vector[EXCP_SYS_NEW].a1 = sysbp_old->a1;
		curProc->excp_state_vector[EXCP_SYS_NEW].a2 = sysbp_old->a2;
		curProc->excp_state_vector[EXCP_SYS_NEW].a3 = sysbp_old->a3;
		curProc->excp_state_vector[EXCP_SYS_NEW].a4 = sysbp_old->a4;
		curProc->excp_state_vector[EXCP_SYS_NEW].a1 &= 0x0FFFFFFF;									// Resetting 4 most significant bit
		curProc->excp_state_vector[EXCP_SYS_NEW].a1 |= ((sysbp_old->cpsr & 0x0000000F) << 28);		// Copying lower cpsr 4 bits to A1 higher 4 bits

		curProc->kernel_mode += getTODLO() - kernelStart;
		LDST(&(curProc->excp_state_vector[EXCP_SYS_NEW]));
	}
}

void pgmHandler() {
	kernelStart = getTODLO();									// Start kernel time interval
	handlerSYSTLBPGM(PGMT_HDL, EXCP_PGMT_NEW, pgmtrap_old);		// Handle Program Trap exception
}
