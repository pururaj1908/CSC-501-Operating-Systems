/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL scount(int sem)
{
	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;	

extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
	{
		if(traceEnable)
		{
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(10,timeElapsed);
		}
	
		return(SYSERR);
	}
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(10,timeElapsed);
	}	

	return(semaph[sem].semcnt);
}
