/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL	sleep10(int n)
{
	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;	
	
	STATWORD ps;    
	if (n < 0  || clkruns==0)
	{
		if(traceEnable)
		{	
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(19,timeElapsed);
		}
	        return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(19,timeElapsed);
	}
	return(OK);
}
