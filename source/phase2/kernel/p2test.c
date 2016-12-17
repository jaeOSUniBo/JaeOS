/*****************************************************************************
 * Copyright 2004, 2005 Michael Goldweber, Davide Brini.                     *
 *                                                                           *
 * This file is part of kaya.                                                *
 *                                                                           *
 * kaya is free software; you can redistribute it and/or modify it under the *
 * terms of the GNU General Public License as published by the Free Software *
 * Foundation; either version 2 of the License, or (at your option) any      *
 * later version.                                                            *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.                  *
 *****************************************************************************/

/*********************************P2TEST.C*******************************
 *
 *	Test program for the Kaya Kernel: phase 2.
 *
 *	Produces progress messages on Terminal0.
 *	
 *	This is pretty convoluted code, so good luck!
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 *      Modified by Davide Brini on Nov 26, 2004
 *      Modified by Renzo Davoli 2010
 * 		Modified by Marco Melletti 2014
 */

//TODO: commenti

#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <const.h>

#include <pcb.h>

typedef unsigned int devregtr;
/* if these are not defined */
typedef unsigned int cpu_t;

/* hardware constants */
#define PRINTCHR	2
#define BYTELEN	8
#define RECVD	5
#define TRANSM 5

#define CLOCKINTERVAL	100000UL	/* interval to V clock semaphore */

#define TERMSTATMASK	0xFF
#define TERMCHARMASK	0xFF00
#define CAUSEMASK		0xFFFFFF
#define VMOFF 			0xF8FFFFFF

#define QPAGE			1024

#define KUPBITON		0x8
#define KUPBITOFF		0xFFFFFFF7

#define MINLOOPTIME		10000
#define LOOPNUM 		10000

#define CLOCKLOOP		10
#define MINCLOCKLOOP		5000	

#define BADADDR			0xFFFFFFFF /* could be 0x00000000 as well */
#define	TERM0ADDR		0x240

/* Software and other constants */
#define PRGOLDVECT		4
#define TLBOLDVECT		2
#define SYSOLDVECT		6

#define CREATENOGOOD	-1

#define TERMINATENOGOOD	-1

#define ON        1
#define OFF       0

#define SYSQUESTION 14
#define SYS

/* just to be clear */
#define SEMAPHORE		int
#define NOLEAVES		4	/* number of leaves of p8 process tree */
#define MAXSEM			20



SEMAPHORE term_mut=1,		/* for mutual exclusion on terminal */
	s[MAXSEM+1],		/* semaphore array */
	testsem=0,		/* for a simple test */
	startp2=0,		/* used to start p2 */
	endp2=0,		/* used to signal p2's demise */
	endp3=0,		/* used to signal p3's demise */
	blkp4=1,		/* used to block second incaration of p4 */
	synp4=0,		/* used to allow p4 incarnations to synhronize */
	endp4=0,		/* to signal demise of p4 */
	blkp5a=0,		
	blkp5b=0,
	blkp5c=0,
	blkp5cint=1,		/* to regulate p5e mode run */
	endp5=0,		/* to signal demise of p5 */
	blkp6=0,		/* used to block p6 */
	endp6=0,		/* to signal demise of p6 */
	endp8=0,		/* to signal demise of p8 */
	endcreate=0,		/* for a p8 leaf to signal its creation */
	blkleaves=0,		/* for a p8 leaf to block */
	blkp8=0;		/* to block p8 */

state_t p2state, p3state, p4state, p5state, p5auxstate, p6state, p7state;
state_t p8rootstate, child1state, child2state;
state_t gchild1state, gchild2state, gchild3state, gchild4state;

int p1p2synch = 0;	/* to check on p1/p2 synchronization */

int p8inc;		/* p8's incarnation number */ 
int p4inc=1;		/* p4 incarnation number */

unsigned int p5hdlstack;	/* so we can allocate new stack for p5 handlers */
unsigned int p5hdlflags;
int p5dmode=0;

int creation = 0; 				/* return code for SYSCALL invocation */
memaddr *p5MemLocation = (memaddr *) 0x34;		/* To cause a p5 trap */

pid_t p3pid;					/* test's and p3's process ids */
pid_t p8pid, leaf1pid, leaf2pid, leaf3pid, leaf4pid;	/* p8 and it's grandchilds' pids */

void	p2(),p3(),p4(),p5(),p5a(),p5b(),p5c(),p6(),p7(),p7a(),p5prog(),p5mm();
void	p5sys(),p6a(),p6b(),p6c(),p8root(),child1(),child2(),p8leaf();

/* a procedure to print on terminal 0 */
void print(char *msg) {

	char * s = msg;
	devregtr command;
	devregtr status;
	
	SYSCALL(SEMOP, (int)&term_mut, -1, 0);				/* get term_mut lock */
	
	while (*s != '\0') {
	  /* Put "transmit char" command+char in term0 register (3rd word). This 
		   actually starts the operation on the device! */
		command = PRINTCHR | (((devregtr) *s) << BYTELEN);
		
		/* Wait for I/O completion (SYS8) */
		status = SYSCALL(IODEVOP, command, INT_TERMINAL, 0);
		
		if ((status & TERMSTATMASK) != TRANSM){
			PANIC();
		}
			
		if (((status & TERMCHARMASK) >> BYTELEN) != *s){
			PANIC();
		}

		s++;	
	}
	
	SYSCALL(SEMOP, (int)&term_mut, 1, 0);				/* release term_mut */
}

void testfun(){}

/*                                                                   */
/*                 p1 -- the root process                            */
/*                                                                   */
void test() {	
	cpu_t		time1, time2;
	pid_t	fpid;

	SYSCALL(SEMOP, (int)&testsem, 1, 0);					/* V(testsem)   */

	if (testsem != 1) { print("error: p1 v(testsem) with no effects\n"); PANIC(); }

	print("p1 v(testsem)\n");

	/* set up states of the other processes */

	/* set up p2's state */
	STST(&p2state);			/* create a state area using p1's state    */
	
	/* stack of p2 should sit above p1's */
	p2state.sp = p2state.sp - QPAGE;
	
	/* p2 starts executing function p2 */
	p2state.pc = (memaddr)p2;
	
	/* p2 has interrupts on and unmasked */
	p2state.cpsr = STATUS_ALL_INT_ENABLE(p2state.cpsr);
		
	
  /* Set up p3's state */
	STST(&p3state);

  /* p3's stack is another 1K below p2's one */
	p3state.sp = p2state.sp - QPAGE;
	p3state.pc = (memaddr)p3;
	p3state.cpsr = STATUS_ALL_INT_ENABLE(p3state.cpsr);
	
	
	/* Set up p4's state */
	STST(&p4state);

  /* p4's stack is another 1k below p3's one */
	p4state.sp = p3state.sp - QPAGE;
	p4state.pc = (memaddr)p4;
	p4state.cpsr = STATUS_ALL_INT_ENABLE(p4state.cpsr);
	
	/* Set up p5's state */
	STST(&p5state);
		
	/* because there will be two p4s running*/
	/* Record the value in p5stack */
	p5state.sp = p4state.sp - (2 * QPAGE);
	p5hdlstack = p5state.sp - QPAGE;
	p5state.pc = (memaddr)p5;
	p5state.cpsr = STATUS_ALL_INT_ENABLE(p5state.cpsr);

	/* Set up p6's states */
	STST(&p6state);
	
	/* p5 will need extra stack pages for subprocesses and handlers */
	p6state.sp = p5hdlstack - (4*QPAGE);
	p6state.pc = (memaddr)p6;
	p6state.cpsr = STATUS_ALL_INT_ENABLE(p6state.cpsr);

	/* Set up p7's state */
	STST(&p7state);
	
	/* Last p6 */
	p7state.sp = p6state.sp - QPAGE;
	p7state.pc = (memaddr)p7;
	p7state.cpsr = STATUS_ALL_INT_ENABLE(p7state.cpsr);

	STST(&p8rootstate);
	p8rootstate.sp = p7state.sp - QPAGE;
	p8rootstate.pc = (memaddr)p8root;
	p8rootstate.cpsr = STATUS_ALL_INT_ENABLE(p8rootstate.cpsr);
    
	STST(&child1state);
	child1state.sp = p8rootstate.sp - QPAGE;
	child1state.pc = (memaddr)child1;
	child1state.cpsr = STATUS_ALL_INT_ENABLE(child1state.cpsr);
	
	STST(&child2state);
	child2state.sp = child1state.sp - QPAGE;
	child2state.pc = (memaddr)child2;
	child2state.cpsr = STATUS_ALL_INT_ENABLE(child2state.cpsr);
	
	STST(&gchild1state);
	gchild1state.sp = child2state.sp - QPAGE;
	gchild1state.pc = (memaddr)p8leaf;
	gchild1state.cpsr = STATUS_ALL_INT_ENABLE(gchild1state.cpsr);

	STST(&gchild2state);
	gchild2state.sp = gchild1state.sp - QPAGE;
	gchild2state.pc = (memaddr)p8leaf;
	gchild2state.cpsr = STATUS_ALL_INT_ENABLE(gchild2state.cpsr);
	
	STST(&gchild3state);
	gchild3state.sp = gchild2state.sp - QPAGE;
	gchild3state.pc = (memaddr)p8leaf;
	gchild3state.cpsr = STATUS_ALL_INT_ENABLE(gchild3state.cpsr);
	
	STST(&gchild4state);
	gchild4state.sp = gchild3state.sp - QPAGE;
	gchild4state.pc = (memaddr)p8leaf;
	gchild4state.cpsr = STATUS_ALL_INT_ENABLE(gchild4state.cpsr);
	
	/* create process p2 */
	SYSCALL(CREATEPROCESS, (int)&p2state, 0, 0);				/* start p2     */

	print("p2 was started\n");

	SYSCALL(SEMOP, (int)&startp2, 1, 0);					/* V(startp2)   */

  /* P1 blocks until p2 finishes and Vs endp2 */
	SYSCALL(SEMOP, (int)&endp2, -1, 0);					/* P(endp2)     */

	/* make sure we really blocked */
	if (p1p2synch == 0)
		print("error: p1/p2 synchronization bad\n");

	p3pid = SYSCALL(CREATEPROCESS, (int)&p3state, 0, 0);				/* start p3  */

	print("p3 is started\n");

  /* P1 blocks until p3 ends */
	SYSCALL(SEMOP, (int)&endp3, -1, 0);					/* P(endp3)     */
	
	print("p1 knows p3 ended\n");
	
	SYSCALL(CREATEPROCESS, (int)&p4state, 0, 0);		/* start p4     */

	SYSCALL(CREATEPROCESS, (int)&p5state, 0, 0); 		/* start p5     */

	SYSCALL(CREATEPROCESS, (int)&p6state, 0, 0);		/* start p6	*/

	SYSCALL(CREATEPROCESS, (int)&p7state, 0, 0);		/* start p7	*/

	SYSCALL(SEMOP, (int)&endp5, -1, 0);
	
	print("p1 knows p5 ended\n");

	SYSCALL(SEMOP, (int)&blkp4, -1, 0);					/* P(blkp4)		*/
	
	print("p1 knows p4 ended\n");

	SYSCALL(SEMOP, (int)&blkp6, 1, 0);

	SYSCALL(SEMOP, (int)&endp6, -1, 0);
	
	/* now for a more rigorous check of process termination */
	SYSCALL(SEMOP, (int)&blkp8, 1, 0);
	
	for (p8inc = 0; p8inc < 4; p8inc++) {
		SYSCALL(SEMOP, (int)&blkp8, -1, 0);

		/* reset leaves lock to 0 for second to fourth runs */
		blkleaves = 0;
		
		creation = SYSCALL(CREATEPROCESS, (int)&p8rootstate, 0, 0);

		if (creation == CREATENOGOOD) {
			print("error in p8 process creation\n");
			PANIC();
		}

		SYSCALL(SEMOP, (int)&endp8, -1, 0);
		
		testfun();

		SYSCALL(TERMINATEPROCESS, (int)p8pid, 0, 0);

		/* do some delay to be reasonably sure p8 and its offspring are dead */
		time1 = 0;
		time2 = 0;
		while (time2 - time1 < (CLOCKINTERVAL >> 1))  {
			time1 = getTODLO();
			SYSCALL(WAITCLOCK, 0, 0, 0);
			time2 = getTODLO();
		}

		SYSCALL(SEMOP, (int)&blkp8, 1, 0);
	}
	
	print("p1 finishes OK -- TTFN\n");
	* ((memaddr *) BADADDR) = 0;				/* terminate p1 */

	/* should not reach this point, since p1 just got a program trap */
	print("error: p1 still alive after progtrap & no trap vector\n");
	PANIC();					/* PANIC !!!     */
}

/* p2 -- semaphore and cputime-SYS test process */
void p2() {
	int		i;				       /* just to waste time  */
	cpu_t	now1,now2;		   /* times of day        */
	cpu_t	usr_t1, usr_t2, glob_t1, glob_t2;	 /* cpu time used       */

  /* startp2 is initialized to 0. p1 Vs it then waits for p2 termination */
	SYSCALL(SEMOP, (int)&startp2, -1, 0);				/* P(startp2)   */

	print("p2 starts\n");

	/* initialize all semaphores in the s[] array */
	for (i = 0; i <= MAXSEM; i++) s[i] = 0;

	/* V, then P, all of the semaphores in the s[] array */
	for (i = 0; i <= MAXSEM; i++)  {
		SYSCALL(SEMOP, (int)&s[i], 1, 0);			/* V(S[I]) */
		SYSCALL(SEMOP, (int)&s[i], -1, 0);			/* P(S[I]) */
		if (s[i] != 0) print("error: p2 bad v/p pairs\n");
	}

	print("p2 v/p pairs successfully\n");

	/* test of SYS6 */

	now1 = getTODLO();                  				/* time of day */
	SYSCALL(GETCPUTIME, (int)&glob_t1, (int)&usr_t1, 0);		/* process time */

	/* delay for several milliseconds */
	for (i = 1; i < LOOPNUM; i++)
		;

	SYSCALL(GETCPUTIME, (int)&glob_t2, (int)&usr_t2, 0);		/* process time */
	now2 = getTODLO();				/* time of day  */

	if (((now2 - now1) >= (usr_t2 - usr_t1)) && 
			((glob_t2 - glob_t1) >= (usr_t2 - usr_t1)) &&
			((usr_t2 - usr_t1) >= MINLOOPTIME)) {
		print("p2 is OK\n");
	} else {
		if ((glob_t2 - glob_t1) < (usr_t2 - usr_t1))
			print ("error: more system time than user time\n");
		if ((now2 - now1) < (usr_t2 - usr_t1))
			print ("error: more cpu time than user time\n");
		if ((usr_t2 - usr_t1) < MINLOOPTIME)
			print ("error: not enough cpu time went by\n");
		print("p2 blew it!\n");
	}

	p1p2synch = 1;				/* p1 will check this */

	SYSCALL(SEMOP, (int)&endp2, 1, 0);				/* V(endp2)     */

	SYSCALL(TERMINATEPROCESS, 0, 0, 0);			/* terminate p2 */

	/* just did a SYS2, so should not get to this point */
	print("error: p2 didn't terminate\n");
	PANIC();					/* PANIC! */
}

/* p3 -- getPid and clock semaphore test process */
void p3() {
	cpu_t	time1, time2;
	cpu_t	usr_t1,usr_t2, glob_t1, glob_t2;	/* cpu time used */
	pid_t	ppid;
	int	i;

	time1 = 0;
	time2 = 0;

	ppid = SYSCALL(GETPID, 0, 0, 0);
	if(ppid != p3pid)
		print("error: p3 pid different from getPid output\n");

	/* loop until we are delayed at least half of clock V interval */
	while ((time2 - time1) < (CLOCKINTERVAL >> 1) )  {
		time1 = getTODLO();		/* time of day     */
		SYSCALL(WAITCLOCK, 0, 0, 0);
		time2 = getTODLO();			/* new time of day */
	}

	print("p3 - WAITCLOCK OK\n");

	/* now let's check to see if we're really charged for CPU
	   time correctly */
	SYSCALL(GETCPUTIME, (int)&glob_t1, (int)&usr_t1, 0);		/* process time */

	for (i = 0; i < CLOCKLOOP; i++)
		SYSCALL(WAITCLOCK, 0, 0, 0);

	SYSCALL(GETCPUTIME, (int)&glob_t2, (int)&usr_t2, 0);		/* process time */
	
	if (((usr_t2 - usr_t1) > MINCLOCKLOOP) || ((glob_t2 - glob_t1) < MINCLOCKLOOP))
		print("error: p3 - CPU time incorrectly maintained\n");
	else
		print("p3 - CPU time correctly maintained\n");

	SYSCALL(SEMOP, (int)&endp3, 1, 0);				/* V(endp3)        */

	SYSCALL(TERMINATEPROCESS, 0, 0, 0);			/* terminate p3    */

	/* just did a SYS2, so should not get to this point */
	print("error: p3 didn't terminate\n");
	PANIC();					/* PANIC  */
}

/* p4 -- termination test process */
void p4() {
	
	switch (p4inc) {
		case 1:
			print("first incarnation of p4 starts\n");
			break;
		case 2:
			print("second incarnation of p4 starts\n");
			break;
	}
	p4inc++;

	SYSCALL(SEMOP, (int)&synp4, 1, 0);				/* V(synp4)     */

	/* first incarnation made blkp4=0, the second is blocked (blkp4 become -1) */
	SYSCALL(SEMOP, (int)&blkp4, -1, 0);				/* P(blkp4)     */

	if(p4inc == 3){
		print("error: second incarnation of p4 did not terminate with original p4\n");
		PANIC();
	}

	SYSCALL(SEMOP, (int)&synp4, -1, 0);				/* P(synp4)     */

	/* start another incarnation of p4 running, and wait for  */
	/* a V(synp4). the new process will block at the P(blkp4),*/
	/* and eventually, the parent p4 will terminate, killing  */
	/* off both p4's.                                         */

	p4state.sp = p4state.sp - QPAGE;		/* give another page  */

	print("p4 create a new p4\n");
	SYSCALL(CREATEPROCESS, (int)&p4state, 0, 0);			/* start a new p4    */

	SYSCALL(SEMOP, (int)&synp4, -1, 0);				/* wait for it       */

	SYSCALL(SEMOP, (int)&endp4, 1, 0);				/* V(endp4)          */

	print("p4 termination with the child\n");
	
	SYSCALL(TERMINATEPROCESS, 0, 0, 0);			/* terminate p4      */

	/* just did a SYS2, so should not get to this point */
	print("error: p4 didn't terminate\n");
	PANIC();					/* PANIC            */
}

/* p5's program trap handler */
void p5prog(unsigned int cause) {
	unsigned int exeCode = cause & CAUSEMASK;
	
	switch (exeCode) {
	case EXC_BUSINVFETCH:
		print("pgmTrapHandler - Access non-existent memory\n");
		SYSCALL(SEMOP, (int)&blkp5a, 1, 0);
		break;
		
	case EXC_RESERVEDINSTR:
		print("pgmTrapHandler - privileged instruction\n");
		SYSCALL(SEMOP, (int)&blkp5cint, 1, 0);
		break;
		
	case EXC_ADDRINVLOAD:
		print("pgmTrapHandler - Address Error: KSegOS w/KU=1\n");
		break;
		
	default:
		print("pgmTrapHandler - other program trap\n");
	}
	
	SYSCALL(TERMINATEPROCESS, 0, 0, 0);

	print("error - PGMT handler did not terminate process\n");
	PANIC();
}

/* p5's memory management (tlb) trap handler */
void p5mm() {
	print("memory management (tlb) trap\n");

	SYSCALL(SEMOP, (int)&blkp5b, 1, 0);

	SYSCALL(TERMINATEPROCESS, 0, 0, 0);

	print("error - TLB handler did not terminate process\n");
	PANIC();
}

/* p5's SYS/BK trap handler */
void p5sys(unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) {
	unsigned int retval = GETPID;
	unsigned int p5status = (a1 >> 28) & 0xF;
	a1 &= 0x0FFFFFFF;

	if(!p5status){
		print("High level SYS call from user mode process\n");
		print("p5c - try to execute return value SYSCALL in user mode (it should be SYS11)\n");
	} else {
		if(a1 == SYSQUESTION){
			retval = (unsigned int) &("the answer is: 42\n");
		}
		print("High level SYS call from kernel mode process\n");
	}

	SYSCALL(EXITTRAP, 0, retval, 0);

	print("error - high level syscall handler did not return\n");
	PANIC();
}

/* p5 -- SYS5 test process */
void p5() {
	cpu_t		time1, time2;

	print("p5 starts\n");
	p5hdlflags = STATUS_ALL_INT_DISABLE(STATUS_NULL | STATUS_SYS_MODE);

	SYSCALL(SEMOP, (int)&endp4, -1, 0);			/* P(endp4)*/

	/* do some delay to be reasonably sure p4 and its offspring are dead */
	time1 = 0;
	time2 = 0;
	while (time2 - time1 < (CLOCKINTERVAL >> 1))  {
		time1 = getTODLO();
		SYSCALL(WAITCLOCK, 0, 0, 0);
		time2 = getTODLO();
	}

	/* if p4 and offspring are really dead, this will increment blkp4 */

	SYSCALL(SEMOP, (int)&blkp4, 2, 0);			/* V(blkp4) */
	
	STST(&p5auxstate);

	p5auxstate.pc = (memaddr)p5a;
	p5auxstate.sp = p5hdlstack - QPAGE;
	SYSCALL(CREATEPROCESS, (int)&p5auxstate, 0, 0);
	SYSCALL(SEMOP, (int)&blkp5a, -1, 0);			/* P(blkp5a) */

	p5auxstate.pc = (memaddr)p5b;
	p5auxstate.sp = p5hdlstack - (2*QPAGE);
	SYSCALL(CREATEPROCESS, (int)&p5auxstate, 0, 0);
	SYSCALL(SEMOP, (int)&blkp5b, -1, 0);			/* P(blkp5b) */

	p5auxstate.pc = (memaddr)p5c;
	p5auxstate.sp = p5hdlstack - (3*QPAGE);
	SYSCALL(CREATEPROCESS, (int)&p5auxstate, 0, 0);
	p5auxstate.sp = p5hdlstack - (4*QPAGE);
	SYSCALL(CREATEPROCESS, (int)&p5auxstate, 0, 0);
	SYSCALL(SEMOP, (int)&blkp5c, -1, 0);			/* P(blkp5c) */

	print("p5 - try to redefine PGMVECT, this will cause p5 termination\n");

	SYSCALL(SEMOP, (int)&endp5, 1, 0);			/* V(endp5) */
	/* should cause a termination       */
	SYSCALL(SPECPGMTHDL, (memaddr)p5prog, p5hdlstack, p5hdlflags);
	SYSCALL(SPECPGMTHDL, (memaddr)p5prog, p5hdlstack, p5hdlflags);

	/* should have terminated, so should not get to this point */
	print("error: p5 didn't terminate\n");
	PANIC();				/* PANIC            */
}

void p5a(){
	SYSCALL(SPECPGMTHDL, (memaddr)p5prog, p5hdlstack, p5hdlflags);

	print("p5a - try to cause a pgm trap accessing some non-existent memory\n");
	/* to cause a pgm trap access some non-existent memory */	
	*p5MemLocation = *p5MemLocation + 1;		 /* Should cause a program trap */

	print("error - p5a still executing\n");
	PANIC();
}

void p5b(){
	unsigned int p5control;

	SYSCALL(SPECTLBHDL, (memaddr)p5mm, p5hdlstack, p5hdlflags);

	print("p5b - try to generate a TLB exception\n");
	
	/* generate a TLB exception by turning on VM without setting up the 
	   seg tables */
	p5control = getCONTROL();
	p5control = p5control | 0x00000001;
	setCONTROL(p5control);

	print("error - p5b still executing\n");
	PANIC();
}

void p5c(){
	char *msg, c;
	int mode;
	unsigned int call;

	SYSCALL(SPECSYSHDL, (memaddr)p5sys, p5hdlstack, p5hdlflags);
	SYSCALL(SPECPGMTHDL, (memaddr)p5prog, p5hdlstack, p5hdlflags);

	SYSCALL(SEMOP, (int)&blkp5cint, -1, 0);
	mode = p5dmode;
	if(!mode){
		p5dmode++;
		print("p5d - try to call SYS14 in user mode\n");
		setSTATUS((getSTATUS() & STATUS_CLEAR_MODE) | STATUS_USER_MODE);
	} else {
		print("p5d - try to call SYS14 in kernel mode\n");
	}

	msg = (char *) SYSCALL(SYSQUESTION, 0, 0, 0);

	if(mode){
		print("p5c - accessing syscall return value\n");
		print(msg);
		SYSCALL(SEMOP, (int)&blkp5c, 1, 0);
		SYSCALL(TERMINATEPROCESS, 0, 0, 0);
	} else {
		SYSCALL((int) msg, 0, 0, 0);
	}

	print("error - p5c still executing\n");
	PANIC();
}

/*p6 -- high level syscall without initializing trap vector */
void p6() {
	print("p6 starts, waiting for other processes to leave terminal\n");
	
	SYSCALL(SEMOP, (int)&blkp6, -2, 0);

	print("p6 - try to request SYS13 without setting trap vector\n");

	SYSCALL(SEMOP, (int)&endp6, 1, 0);

	SYSCALL(13, 0, 0, 0);		/* should cause termination because p6 has no 
					   trap vector */

	print("error: p6 alive after SYS13() with no trap vector\n");
	PANIC();
}

/*p7 -- program trap without initializing passup vector*/
void p7() {
	print("p7 starts\n");

	print("p7 - generating a program trap without setting trap vector\n");

	SYSCALL(SEMOP, (int)&blkp6, 1, 0);

	* ((memaddr *) BADADDR) = 0;
		
	print("error: p7 alive after program trap with no trap vector\n");
	PANIC();
}

/* p8root -- test of termination of subtree of processes              */
/* create a subtree of processes, wait for the leaves to block, signal*/
/* the root process, and then terminate                               */
void p8root() {
	int i;
	
	p8pid = SYSCALL(GETPID, 0, 0, 0);

	print("p8root starts\n");

	SYSCALL(CREATEPROCESS, (int)&child1state, 0, 0);

	SYSCALL(CREATEPROCESS, (int)&child2state, 0, 0);

	SYSCALL(SEMOP, (int)&endcreate, -(NOLEAVES), 0);
	
	SYSCALL(TERMINATEPROCESS, (int)leaf1pid, 0, 0);
	SYSCALL(TERMINATEPROCESS, (int)leaf2pid, 0, 0);
	SYSCALL(TERMINATEPROCESS, (int)leaf3pid, 0, 0);
	SYSCALL(TERMINATEPROCESS, (int)leaf4pid, 0, 0);

	SYSCALL(SEMOP, (int)&blkleaves, NOLEAVES, 0);

	SYSCALL(SEMOP, (int)&endp8, 1, 0);

	SYSCALL(SEMOP, (int)&blkp8, -1, 0);
}

/*child1 & child2 -- create two sub-processes each*/

void child1() {
	print("child1 starts\n");
	
	leaf1pid = SYSCALL(CREATEPROCESS, (int)&gchild1state, 0, 0);
	
	leaf2pid = SYSCALL(CREATEPROCESS, (int)&gchild2state, 0, 0);

	SYSCALL(SEMOP, (int)&blkp8, -1, 0);
	
	print("error: p8 child was not killed with father\n");
	PANIC();
}

void child2() {
	print("child2 starts\n");

	leaf3pid = SYSCALL(CREATEPROCESS, (int)&gchild3state, 0, 0);
	
	leaf4pid = SYSCALL(CREATEPROCESS, (int)&gchild4state, 0, 0);

	SYSCALL(SEMOP, (int)&blkp8, -1, 0);
	
	print("error: p8 child was not killed with father\n");
	PANIC();
}

/*p8leaf -- code for leaf processes*/

void p8leaf() {
	print("leaf process starts\n");

	SYSCALL(SEMOP, (int)&endcreate, 1, 0);

	SYSCALL(SEMOP, (int)&blkleaves, -1, 0);
	
	print("error: p8 grandchild was not killed with father\n");
	PANIC();
}
