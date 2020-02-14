/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	unsigned long ctr1000;
    	unsigned long timeElapsed = ctr1000;
	
	short	*nxtdev;

	if (isbadpid(pid))
	{
		if(traceEnable)
		{
			timeElapsed = ctr1000 - timeElapsed;
			calcSysCallAccounting(13,timeElapsed);
		}

		return(SYSERR);
	}
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	
	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(13,timeElapsed);
	}
	
	return(OK);
}
