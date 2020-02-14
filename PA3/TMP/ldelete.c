#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


SYSCALL ldelete(int lockdescriptor)
{
	STATWORD ps;    
	int	pid;
	struct	lentry	*lptr;
	int i;

	disable(ps);
	if (isbadlock(lockdescriptor) || rw_locks[lockdescriptor].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &rw_locks[lockdescriptor];
	lptr->lstate = LFREE;
	lptr->ltype = DELETED;
	lptr->lprio = -1;
	/* reset bit mask of process ids currently holding the lock */
	for (i=0;i<NPROC;i++)
	{
		if (lptr->lproc_list[i] == 1)
		{
			lptr->lproc_list[i] = 0;
			proctab[i].bm_locks[lockdescriptor] = 0;
		}
	}	
	
	if (nonempty(lptr->lqhead)) {
		while( (pid=getfirst(lptr->lqhead)) != EMPTY)
		  {
		    proctab[pid].plockret = DELETED;
		    proctab[pid].wait_lockid = -1;	
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
