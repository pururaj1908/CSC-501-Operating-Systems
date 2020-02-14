/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <lab0.h>
extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */

    unsigned long timeElapsed;
    unsigned long ctr1000;

    if(traceEnable)
    {
         timeElapsed = ctr1000;
   	 timeElapsed = ctr1000 - timeElapsed;
         calcSysCallAccounting(4,timeElapsed);
    }
		

    return OK;
}
