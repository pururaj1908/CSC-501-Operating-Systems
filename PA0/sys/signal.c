/* signal.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * signal  --  signal a semaphore, releasing one waiting process
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL signal(int sem)
{
	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;
	
	STATWORD ps;    
	register struct	sentry	*sptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		
		if(traceEnable)
		{	
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(16,timeElapsed);
		}	
		return(SYSERR);
	}
	if ((sptr->semcnt++) < 0)
		ready(getfirst(sptr->sqhead), RESCHYES);
	restore(ps);
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(16,timeElapsed);
	}
	return(OK);
}
