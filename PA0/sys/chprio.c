/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL chprio(int pid, int newprio)
{
        unsigned long ctr1000;
        unsigned long timeElapsed = ctr1000;	
	
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		 if(traceEnable)
        	{
                	timeElapsed = ctr1000 - timeElapsed;
                	calcSysCallAccounting(1,timeElapsed);
        	}

		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);

	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
                calcSysCallAccounting(1,timeElapsed);
	}

	return(newprio);
}
