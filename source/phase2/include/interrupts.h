/*
 *	INTERRUPTS.H
 *	Device Interrupt Exception Handlers Header File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <initial.h>


void intHandler();					// Entry point function of interrupt management
void deviceHandler(int type);		// Device handler function; type indicates device type that caused interrupt (see uARMconst.h for details)
void terminalHandler();				// Terminal (tx/rx) handler function
void timerHandler();				// Timer handler function resposible of pseudo clock and time slice management

#endif
