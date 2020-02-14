/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL getpid()
{
	unsigned long timeElapsed;
	unsigned long ctr1000;
	
	if(traceEnable)
	{
		timeElapsed = ctr1000;
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(2,timeElapsed);
	}
	return(currpid);
}
