/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL getprio(int pid)
{
        unsigned long ctr1000;
        unsigned long timeElapsed = ctr1000;		

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(traceEnable)
        	{
                	timeElapsed = ctr1000 - timeElapsed;
                	calcSysCallAccounting(3,timeElapsed);
       		}

		return(SYSERR);
	}
	restore(ps);
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(3,timeElapsed);
	}
	
	return(pptr->pprio);
}
