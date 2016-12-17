/*
 *	SYSCALL.C
 *	System Calls Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include <syscall.h>

/***************************************************************
*                      AUXILIARY FUNCTION                      *
***************************************************************/

void saveCurState(state_t *state, state_t *newState) {
	newState->a1 = state->a1;
	newState->a2 = state->a2;
	newState->a3 = state->a3;
	newState->a4 = state->a4;
	newState->v1 = state->v1;
	newState->v2 = state->v2;
	newState->v3 = state->v3;
	newState->v4 = state->v4;
	newState->v5 = state->v5;
	newState->v6 = state->v6;
	newState->sl = state->sl;
	newState->fp = state->fp;
	newState->ip = state->ip;
	newState->sp = state->sp;
	newState->lr = state->lr;
	newState->pc = state->pc;
	newState->cpsr = state->cpsr;
	newState->CP15_Control = state->CP15_Control;
	newState->CP15_EntryHi = state->CP15_EntryHi;
	newState->CP15_Cause = state->CP15_Cause;
	newState->TOD_Hi = state->TOD_Hi;
	newState->TOD_Low = state->TOD_Low;
}

pid_t genPid(UI a) {
	UI count = getTODLO();  // pseudo random generator
	count &= 0x0000FFFF;
	count <<= 16;
	return a + count;
}

pcb_t* searchPid(pcb_t *parent, const pid_t pid) {
	void* tmp = NULL;
	pcb_t* scan = NULL;
	pcb_t* save = NULL;

	// Scan parent progeny finding process whose identifier is PID
	clist_foreach(scan, &(parent->p_children), p_siblings, tmp) {
		if (scan->pid == pid) return scan;
		else {
			if (!emptyChild(scan)) save = searchPid(scan, pid);
			if (save) return save;
		}
	}

	return NULL;
}

void terminator(pcb_t* proc) {
	while (!emptyChild(proc)) terminator(removeChild(proc));	// terminate PROC progeny

	if (proc->p_cursem != NULL) {
		// Update semaphore value when process isn't blocked on a device sempahore; otherwise softBlockCounter gets updated
		if ((int *)(proc->p_cursem) >= &(semDevices[0]) && (int *)(proc->p_cursem) <= &(semDevices[MAXPROC-1])) softBlockCounter--;
		else updateSemaphoreValue(proc->p_cursem, proc->waitingResCount);

		outBlocked(proc);
	}

	processCounter--;

	outProcQ(&readyQueue, proc);
	outChild(proc);
	freePcb(proc);
}

void setSYSTLBPGMT(hdl_type old, UI new, memaddr handler, memaddr stack, UI flags) {
	UI asid;

	if (old == SYS_HDL) {
		// if SYS/BP handler has already been set, act as SYS 2
		if (curProc->tags == 1 || curProc->tags == 3 || curProc->tags == 5 || curProc->tags == 7) terminateProcess(0);
		else curProc->tags ^= 1;
	}
	else if (old == TLB_HDL) {
		// if TLB handler has already been set, act as SYS 2
		if (curProc->tags == 2 || curProc->tags == 3 || curProc->tags == 6 || curProc->tags == 7) terminateProcess(0);
		else curProc->tags ^= 2;
	}
	else if (old == PGMT_HDL) {
		// if PGMT handler has already been set, act as SYS 2
		if (curProc->tags == 4 || curProc->tags == 5 || curProc->tags == 6 || curProc->tags == 7) terminateProcess(0);
		else curProc->tags ^= 4;
	}
	else PANIC();

	if (curProc) {
		saveCurState(&curProc->p_s, &curProc->excp_state_vector[old]);
		saveCurState(&curProc->p_s, &curProc->excp_state_vector[new]);

		curProc->excp_state_vector[new].pc = handler;						// setting program counter to handler code
		curProc->excp_state_vector[new].sp = stack;							// setting stack
		curProc->excp_state_vector[new].cpsr |= (flags & 0x80000007);		// setting processor execution mode and virtual memory flag

		asid = ENTRYHI_ASID_GET(curProc->excp_state_vector[new].CP15_EntryHi);	// Setting ASID as the same as the current process one
		ENTRYHI_ASID_SET(curProc->excp_state_vector[new].CP15_EntryHi, asid);
	}
}

/***************************************************************
*                     SYSCALL KERNEL MODE                      *
***************************************************************/

int createProcess(state_t *stato) {
	pcb_t *newProc = allocPcb();

	if (newProc == NULL) return -1; // fail

	saveCurState(stato, &(newProc->p_s));

	processCounter++;

	newProc->pid = genPid(newProc->pid);
	insertChild(curProc, newProc);
	insertProcQ(&readyQueue, newProc);

	return newProc->pid; // Success
}

void terminateProcess(pid_t p) {
	pcb_t *save = NULL;

	// Autotermination
	if (p == 0 || curProc->pid == p) {
		terminator(curProc);
		curProc = NULL;
	}
	else {
		if (!(save = searchPid(curProc, p))) PANIC();
		terminator(save);
	}
}

void semaphoreOperation(int *sem, int weight) {
	if (!sem) PANIC();

	// Resource to be freed (Verhogen operation)
	if (weight >= 1) {
		(*sem) += weight;

		pcb_t *firstBlocked;

		// Updating waiting resources count to blocked processes until weight resources are enough, so that they can be unblocked
		while ((firstBlocked = headBlocked(sem)) && firstBlocked->waitingResCount <= weight) {
			firstBlocked = outBlocked(firstBlocked);
			// Update softBlockCounter if process is blocked on a device semaphore
			if (sem >= &semDevices[0] && sem <= &semDevices[MAX_DEVICES-1]) softBlockCounter--;		
			weight -= firstBlocked->waitingResCount;
			firstBlocked->waitingResCount = 0;
			insertProcQ(&readyQueue, firstBlocked);
		}
		firstBlocked->waitingResCount -= weight;
	}
	// Resources to be allocated (Passeren operation)
	else if (weight <= -1) {
		(*sem) += weight;	// updating semaphore value

		if (*sem < 0) {
			curProc->waitingResCount = -weight;		// process needs "weight" resources

			// updating timing
			curProc->kernel_mode += getTODLO() - kernelStart;
			curProc->global_time += getTODLO() - processStart;
			curProc->remaining -= getTODLO() -processStart;

			if (insertBlocked(sem, curProc)) PANIC();
			// Update softBlockCounter if process blocks on a device semaphore
			if (sem >= &semDevices[0] && sem <= &semDevices[MAX_DEVICES-1]) softBlockCounter++;
			curProc = NULL;
		}
	}
	else terminateProcess(curProc->pid);
}

void specifySysBp(memaddr handler, memaddr stack, UI flags) {
	setSYSTLBPGMT(SYS_HDL, EXCP_SYS_NEW, handler, stack, flags);
}

void specifyTlb(memaddr handler, memaddr stack, UI flags) {
	setSYSTLBPGMT(TLB_HDL, EXCP_TLB_NEW, handler, stack, flags);
}

void specifyPgm(memaddr handler, memaddr stack, UI flags) {
	setSYSTLBPGMT(PGMT_HDL, EXCP_PGMT_NEW, handler, stack, flags);
}

void exitTrap(hdl_type exType, UI ret) {
	// Returns return value to process and load it from where it suspended
	switch(exType) {
		case SYS_HDL:
			curProc->excp_state_vector[EXCP_SYS_OLD].a1 = ret;
			LDST(&curProc->excp_state_vector[EXCP_SYS_OLD]);
			break;
		case TLB_HDL:
			curProc->excp_state_vector[EXCP_TLB_OLD].a1 = ret;
			LDST(&curProc->excp_state_vector[EXCP_TLB_OLD]);
			break;
		case PGMT_HDL:
			curProc->excp_state_vector[EXCP_PGMT_OLD].a1 = ret;
			LDST(&curProc->excp_state_vector[EXCP_PGMT_OLD]);
			break;
		default:
			PANIC();
	}
}

void getCpuTime(cputime_t *global_time, cputime_t *user_time) {
	// update process timing
	curProc->global_time += getTODLO() - processStart;
	curProc->kernel_mode += getTODLO() - kernelStart;

	*global_time = curProc->global_time;
	*user_time = curProc->global_time - curProc->kernel_mode;
}

void waitForClock() {
	int index = MAX_DEVICES - 1;					// pseudo clock semaphore is the last one in the array
	semaphoreOperation(&semDevices[index], -1);		// lock the current process on pseudo clock semaphore
}

UI iodevop(UI command, int lineNum, UI deviceNum) {
	devreg_t *deviceRegister = (devreg_t *)DEV_REG_ADDR(lineNum, deviceNum);	// Getting device register from interrupt line and device number

	// Checking if is a terminal read operation
	UI terminalReading = (lineNum == INT_TERMINAL && deviceNum >> 31) ? N_DEV_PER_IL : 0;
	// Calculating sempahore index associated to specific device
	int index = EXT_IL_INDEX(lineNum) * N_DEV_PER_IL;
	index += deviceNum;
	index += terminalReading;

	semaphoreOperation(&semDevices[index], -1);

	UI status = 0;
	if (lineNum == INT_TERMINAL) {
		if (terminalReading > 0) {		// Terminal read operation
			deviceRegister->term.recv_command = command;
			status = deviceRegister->term.recv_status;
		}
		else {		// terminal write operation
			deviceRegister->term.transm_command = command;
			status = deviceRegister->term.transm_status;
		}
	}
	else {		// other devices management
		deviceRegister->dtp.command = command;
		status = deviceRegister->dtp.status;
	}
	return status;
}

pid_t getPid() {
	return curProc->pid;
}
