#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int releaseall (int numlocks, long lks,...)
{

	STATWORD ps;
	struct lentry *lptr;
	struct pentry *pptr;
	disable(ps);
	
	int i;
	int ld;
        unsigned long *a; 
	int flag = 0;
	pptr = &proctab[currpid];
	
	a = (unsigned long *)(&lks);
	for (i=0;i<numlocks;i++)
	{
		ld = *a++;

		//kprintf("%d",ld);
		/* check if lock descriptor passed is valid or not and is held by the calling process */
		if (isbadlock(ld)) 
		{
               		flag = 1;	   	
       		}
		else
		{
			lptr = &rw_locks[ld];
			if (lptr->lproc_list[currpid] == 1)
			{
				releaseLDForProc(currpid, ld);					
			}
			else
			{
				flag = 1;
			} 
		}		
	}

	resched();

	restore(ps);
	return flag == 0 ? OK : SYSERR;	
}

void releaseLDForProc(int pid, int ld)
{
	
	struct lentry *lptr;
	struct pentry *pptr;
	struct pentry *nptr;
	struct pentry *wptr;
	
	int oltype = lptr->ltype;
	int maxprio = -1;
	int i=0;

	lptr = &rw_locks[ld];
	pptr = &proctab[pid];

	/* set ltype deleted temporarily */
	lptr->ltype = DELETED;
	lptr->lproc_list[pid] = 0;
	
	/* release ld lock from bit mask */
	pptr->bm_locks[ld] = 0;
	pptr->wait_lockid = -1;
	pptr->wait_ltype = -1;

	if (nonempty(lptr->lqhead))
	{
		int prev = lptr->lqtail;
		int writerProcExist = 0;
		int readerProcHoldingLock = 0;
		int wpid = 0;
		struct qent *mptr;
		unsigned long tdf = 0;	
		maxprio = q[q[prev].qprev].qkey;
		
		/* check writer proc exist in queue  */
		while (q[prev].qprev != lptr->lqhead)
		{
			prev = q[prev].qprev;
			wptr = &proctab[prev];
			if (wptr->wait_ltype == WRITE)
			{
				writerProcExist = 1;
				wpid = prev;
				mptr = &q[wpid];
				break;		
			}	
		}
		
		if (writerProcExist == 0)
		{
			prev = lptr->lqtail;
			
			while (q[prev].qprev != lptr->lqhead && q[prev].qprev < NPROC && q[prev].qprev > 0)
			{	
				prev = q[prev].qprev;
				dequeue(prev);

				nptr = &proctab[prev];
				nptr->bm_locks[ld] = 1;

 				lptr->ltype = READ;
				lptr->lproc_list[prev] = 1;
				nptr->wait_time = 0;
				nptr->wait_lockid = -1;
				nptr->wait_ltype = -1;

				ready(prev, RESCHNO);
			}	
		}
		else if (writerProcExist == 1)
		{
			prev = lptr->lqtail;
			if (mptr->qkey == maxprio)
			{
				tdf = proctab[q[prev].qprev].wait_time - wptr->wait_time;
				if (tdf < 0)
				{
					tdf = (-1)*tdf; /* make time difference positive */
				}
				if (tdf < 1000) /* within 1 s */
				{
					for (i = 0;i < NPROC;i++)
					{
						if (lptr->lproc_list[i] == 1)
						{
							readerProcHoldingLock = 1;
							break;
						}
					}
					if (readerProcHoldingLock == 0)
					{
					
						dequeue(wpid);
	
						nptr = &proctab[wpid];
						nptr->bm_locks[ld] = 1;
	
 						lptr->ltype = WRITE;
						lptr->lproc_list[wpid] = 1;
						nptr->wait_time = 0;
						nptr->wait_lockid = -1;
						nptr->wait_ltype = -1;

						ready(wpid, RESCHNO);
					}
				}
				else
				{
					prev = lptr->lqtail;
					while (q[prev].qprev != wpid)
					{
						prev = q[prev].qprev;
						dequeue(prev);

						nptr = &proctab[prev];
						nptr->bm_locks[ld] = 1;

 						lptr->ltype = READ;
						lptr->lproc_list[prev] = 1;
						nptr->wait_time = 0;
						nptr->wait_lockid = -1;
						nptr->wait_ltype = -1;

						ready(prev, RESCHNO);
					}				
				}
			}
			else
			{
				prev = lptr->lqtail;
				while (q[prev].qprev != wpid)
				{
					prev = q[prev].qprev;
					dequeue(prev);

					nptr = &proctab[prev];
					nptr->bm_locks[ld] = 1;

 					lptr->ltype = READ;
					lptr->lproc_list[prev] = 1;
					nptr->wait_time = 0;
					nptr->wait_lockid = -1;
					nptr->wait_ltype = -1;

					ready(prev, RESCHNO);
				}
			}
		}
				
	}
		
	lptr->lprio = getMaxPriorityInLockWQ(ld);
	maxprio = getMaxWaitProcPrioForPI(pid);
	
	if (maxprio > pptr->pprio)
	{
		pptr->pinh = maxprio;
	}
	else
	{
		pptr->pinh = 0; /* as maxprio is either equal or less than original priority of pptr process */
	}
	
			
}

void releaseLDForWaitProc(pid, ld)
{
	struct lentry *lptr;
	struct pentry *pptr;
	
	lptr = &rw_locks[ld];
	pptr = &proctab[pid];

	dequeue(pid);
	pptr->wait_lockid = -1;
	pptr->wait_ltype = -1;
	pptr->wait_time = 0;
	pptr->plockret = DELETED;

	lptr->lprio = getMaxPriorityInLockWQ(ld);	
	rampUpProcPriority(ld,-1);
}
