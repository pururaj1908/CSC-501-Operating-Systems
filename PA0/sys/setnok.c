/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL	setnok(int nok, int pid)
{
	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;
	
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
			
		if(traceEnable)
		{
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(14,timeElapsed);
		}
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(14,timeElapsed);
	}
	return(OK);
}
