/*
 *	INTERRUPTS.C
 *	Device Interrupt Exception Handlers Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#include <interrupts.h>

void intHandler() {
	state_t *oldState = (state_t *)INT_OLDAREA;
	if (curProc) {
		(*oldState).pc -= WORD_SIZE;				// Update Program Counter to instruction that caused interrupt invocation
		saveCurState(oldState, &curProc->p_s);
	}

	UI cause = getCAUSE();

	// Analyze interrupt line to understand which kind of device caused the interrupt
	if (CAUSE_IP_GET(cause, IL_TIMER)) timerHandler();
	else if (CAUSE_IP_GET(cause, IL_DISK)) deviceHandler(IL_DISK);
	else if (CAUSE_IP_GET(cause, IL_TAPE)) deviceHandler(IL_TAPE);
	else if (CAUSE_IP_GET(cause, IL_ETHERNET)) deviceHandler(IL_ETHERNET);
	else if (CAUSE_IP_GET(cause, IL_PRINTER)) deviceHandler(IL_PRINTER);
	else if (CAUSE_IP_GET(cause, IL_TERMINAL)) terminalHandler();

	scheduler();
}

UI getDeviceNumberFromLineBitmap(int *lineAddr) {
	int activeBit = 0x00000001;
	int i;

	// Analyze the interrupt line to check which specific device has caused the interrupt
	for (i = 0; i < 8; i++) {
		if ((*lineAddr & activeBit) == activeBit) break;
		activeBit = activeBit << 1;
	}

	return i;
}

void acknowledge(UI semIndex, devreg_t *devRegister, ack_type type) {
	// if a process is blocked on a device, it gets the result of operation and acknowledge operation end to the device 
	if (semDevices[semIndex] < 1) {
		pcb_t *process = headBlocked(&semDevices[semIndex]);
		switch (type) {
			case ACK_GEN_DEVICE:
				process->p_s.a1 = devRegister->dtp.status;
				devRegister->dtp.command = DEV_C_ACK;
				break;
			case ACK_TERM_TRANSMIT:
				process->p_s.a1 = devRegister->term.transm_status;
				devRegister->term.transm_command = DEV_C_ACK;
				break;
			case ACK_TERM_RECEIVE:
				process->p_s.a1 = devRegister->term.recv_status;
				devRegister->term.recv_command = DEV_C_ACK;
				break;
			default:
				PANIC();
				break;
		}
		semaphoreOperation(&semDevices[semIndex], 1);
	}
}

void deviceHandler(int type) {
	UI *bitmapForLine = (UI *)CDEV_BITMAP_ADDR(type);								// Getting interrupt line bitmat for specific device type
	UI deviceNumber = getDeviceNumberFromLineBitmap(bitmapForLine);					// Getting device number from interrupt line bitmap
	devreg_t *deviceRegister = (devreg_t *)DEV_REG_ADDR(type, deviceNumber);		// Getting device register of the device to be managed

	UI index = EXT_IL_INDEX(type) * N_DEV_PER_IL + deviceNumber;					// Calculating semaphore index associated to device
	acknowledge(index, deviceRegister, ACK_GEN_DEVICE);
}

void terminalHandler() {
	UI *bitmapForLine = (UI *)CDEV_BITMAP_ADDR(INT_TERMINAL);
	UI terminalNumber = getDeviceNumberFromLineBitmap(bitmapForLine);
	devreg_t *terminalRegister = (devreg_t *)DEV_REG_ADDR(INT_TERMINAL, terminalNumber);
	UI index = 0;
	
	// If a characher has been transmitted (higher priority operation then receiving)
	if ((terminalRegister->term.transm_status & 0x0000000F) == DEV_TTRS_S_CHARTRSM) {
		index = EXT_IL_INDEX(INT_TERMINAL) * N_DEV_PER_IL + terminalNumber;
		acknowledge(index, terminalRegister, ACK_TERM_TRANSMIT);
	}
	// If a character has been been received
	else if ((terminalRegister->term.recv_status & 0x0000000F) == DEV_TRCV_S_CHARRECV) {
		index = EXT_IL_INDEX(INT_TERMINAL) * N_DEV_PER_IL + N_DEV_PER_IL + terminalNumber;
		acknowledge(index, terminalRegister, ACK_TERM_RECEIVE);
	}
}

void timerHandler() {
	clock += getTODLO() - clockTick;		// Update psudo clock time and initialize new interval (clockTick)
	clockTick = getTODLO();

	// 100ms have expired
	if (clock >= SCHED_PSEUDO_CLOCK) {
		// Unblock processes waiting for timer
		while (semDevices[MAX_DEVICES-1] < 0) {
			semaphoreOperation(&semDevices[MAX_DEVICES-1], 1);
		}

		// Initialize clock with a new value considering occurred delays
		clock = -(SCHED_PSEUDO_CLOCK - clock);
		clockTick = getTODLO();
	}
	// Current process has ran out his time slice
	if (curProc) {
		curProc->global_time += getTODLO() - processStart;		// updating process global time
		insertProcQ(&readyQueue, curProc);						// process in ready queue
		curProc->remaining = SCHED_TIME_SLICE;					// resetting time slice
		curProc = NULL;

		clock += getTODLO() - clockTick;
		clockTick = getTODLO();
	}
	setTIMER(SCHED_TIME_SLICE);
}
