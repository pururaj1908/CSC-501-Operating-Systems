#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

struct  lentry  rw_locks[NLOCKS];

void linit()
{

	struct  lentry  *lptr;
	nextlock = NLOCKS-1;
	
	int i;
	int j;
	
	for (i=0;i<NLOCKS;i++) /* initialize lock descriptors */
	{
		lptr = &rw_locks[i];
		lptr->lstate = 	LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
		lptr->ltype = DELETED;
		lptr->lprio = -1;
		
		/* initialize list maintained for processes holding the lock */
		for (j=0;j<NPROC;j++)
		{
			lptr->lproc_list[j] = 0;
		}	
	}

}
