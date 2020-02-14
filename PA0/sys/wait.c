/* wait.c - wait */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * wait  --  make current process wait on a semaphore
 *------------------------------------------------------------------------
 */

int traceEnable;
SYSCALL	wait(int sem)
{

	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;
		

	STATWORD ps;    
	struct	sentry	*sptr;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);

		if(traceEnable)
		{
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(26,timeElapsed);
		}
		return(SYSERR);
	}
	
	if (--(sptr->semcnt) < 0) {
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		pptr->psem = sem;
		enqueue(currpid,sptr->sqtail);
		pptr->pwaitret = OK;
		resched();
		restore(ps);

		if(traceEnable)
		{
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(26,timeElapsed);
		}
		return pptr->pwaitret;
	}
	restore(ps);

	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(26,timeElapsed);
	}
	return(OK);
}
