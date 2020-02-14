#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int lock (int ldes1, int type, int priority)
{
	STATWORD ps;
	struct lentry *lptr;
	struct pentry *pptr;
	disable(ps);
	
	if (isbadlock(ldes1) || (lptr= &rw_locks[ldes1])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	pptr = &proctab[currpid];

        /* ltype = DELETED means it has not been acquired by any process yet for Read/Write but is created as lstate = LUSED  */
	/* thus any process could get the lock */
	if (lptr->ltype == DELETED)
	{
		lptr->ltype = type;
		lptr->lprio = -1;
		lptr->lproc_list[currpid] = 1;

		pptr->bm_locks[ldes1] = 1; /* set bit mask corresponding to lock desc to 1 for the current process */
		pptr->wait_lockid = -1; /* as it acquires lock it will take value -1  */
		pptr->wait_pprio = priority; /* waiting priority for the lock queue */
		pptr->wait_ltype = -1; /* as the current process is not being blocked */	 	
	}
	
	/* already some process has acquired READ lock on the descriptor  */
	else if (lptr->ltype == READ)
	{
		/* current process wants to acquire WRITE lock */
		if (type == WRITE)
		{
			/* block the current process as WRITE lock is exclusive */
			pptr->pstate = PRWAIT;
			pptr->wait_lockid = ldes1;   /* process waiting on this lock id*/ 
			pptr->wait_time = ctr1000;   /* waiting time start */	
			pptr->wait_pprio = priority; /* waiting priority for the lock queue */
			pptr->wait_ltype = type; 

			insert(currpid, lptr->lqhead, priority); /* insert the proc in wait queue of lock descriptor based on its wait priority */
			lptr->lprio = getMaxPriorityInLockWQ(ldes1); /* set lprio field to max priority process in wait queue of the lock */
			rampUpProcPriority(ldes1,getProcessPriority(pptr));

			pptr->plockret = OK;
			resched();
			restore(ps);
			return pptr->plockret;
		}		
				
		/* current process wants to acquire READ lock */
		else if (type == READ)
		{
			/* check whether any higher write priority process is there in lock's wait queue */
			int writerProcExist = 0;
			int next = lptr->lqhead;
			struct pentry *wptr;
			
			while (q[next].qnext != lptr->lqtail)
			{
				next = q[next].qnext;
				wptr = &proctab[next];
				if (wptr->wait_ltype == WRITE && q[next].qkey > priority)
				{
					writerProcExist = 1;
					break;	
				}	
			}
			
			/* there is no writer process having priority more than current process so assign READ lock */
			if (writerProcExist == 0)
			{
				lptr->ltype = type;
				lptr->lprio = getMaxPriorityInLockWQ(ldes1); /* set lprio field to max priority process in wait queue of the lock */
				lptr->lproc_list[currpid] = 1;

				pptr->bm_locks[ldes1] = 1; /* set bit mask corresponding to lock desc to 1 for the current process */
				pptr->wait_lockid = -1; /* as it acquires lock it will take value -1  */
				pptr->wait_pprio = priority; /* waiting priority for the lock queue */
				pptr->wait_ltype = -1; /* as the current process is not being blocked */
				rampUpProcPriority (ldes1,-1);	
			}
			
			/* there is high wait priority writer process in queue so block current process */
			if (writerProcExist == 1)
			{
				/* block the current process*/
				pptr->pstate = PRWAIT;
				pptr->wait_lockid = ldes1;   /* process waiting on this lock id*/ 
				pptr->wait_time = ctr1000;   /* waiting time start */	
				pptr->wait_pprio = priority; /* waiting priority for the lock queue */
				pptr->wait_ltype = type; 

				insert(currpid, lptr->lqhead, priority); /* insert the process in wait queue of lock based on its wait priority */
				lptr->lprio = getMaxPriorityInLockWQ(ldes1); /* set lprio field to max priority process in wait queue of the lock */			               rampUpProcPriority(ldes1,getProcessPriority(pptr)); 		 
				
				pptr->plockret = OK;
				resched();
				restore(ps);
				return pptr->plockret;	
			}
		}			
	}

	/* already some process has acquired WRITE lock on the descriptor */
	else if (lptr->ltype == WRITE)
	{
		/* block the current process as WRITE lock is exclusive */
		pptr->pstate = PRWAIT;
		pptr->wait_lockid = ldes1;   /* process waiting on this lock id*/ 
		pptr->wait_time = ctr1000;   /* waiting time start */	
		pptr->wait_pprio = priority; /* waiting priority for the lock queue */
		pptr->wait_ltype = type; 

		insert(currpid, lptr->lqhead, priority); /* insert the process in wait queue of lock descriptor based on its wait priority */
		lptr->lprio = getMaxPriorityInLockWQ(ldes1); /* set lprio field to max priority process in wait queue of the lock */
		rampUpProcPriority(ldes1,getProcessPriority(pptr)); 
		
		pptr->plockret = OK;
		resched();
		restore(ps);
		return pptr->plockret;
		
	}
	
	restore(ps);
	return(OK); 
}

int getMaxPriorityInLockWQ (int ld)
{
	struct lentry *lptr;
	struct pentry *pptr;

	int lprio = -1;
	int gprio = -1;	
	lptr = &rw_locks[ld];
	
	int next = lptr->lqhead;
	while (q[next].qnext != lptr->lqtail)
	{
		next = q[next].qnext;
		pptr = &proctab[next];
		
		gprio = getProcessPriority(pptr); 
		if (lprio < gprio)
		{
			lprio = gprio;
		} 		
	}

	return lprio;				
}

void rampUpProcPriority (int ld, int priority)
{
	struct lentry *lptr;
	struct pentry *pptr;
	int i;
	int tmpld;
	int gprio = -1;
	int maxprio = -1;				
	lptr = &rw_locks[ld];

	for (i=0;i<NPROC;i++)
	{
		if (lptr->lproc_list[i] == 1)
		{
			pptr = &proctab[i];
			gprio = getProcessPriority(pptr);
			
			/* priority will be -1 when called this func for priority inheritance but when called during lock func it will be */
			/* priority of process currently blocked in lock func */
		 	
			if (priority == -1)
			{
				maxprio = getMaxWaitProcPrioForPI(i);
				
				
				if (maxprio > pptr->pprio)
				{
					pptr->pinh = maxprio;
				}
				else
				{
					pptr->pinh = 0; /* as maxprio is either equal or less than original priority of pptr process */
				}
				
				tmpld = checkProcessTransitivityForPI(i);
				if (tmpld != -1)
				{
					rampUpProcPriority (tmpld,-1);
				}			 
			}
			
			/* ramp up the priority of pptr upto priority passed */
			else if (gprio < priority)
			{
				pptr->pinh = priority;
				tmpld = checkProcessTransitivityForPI(i);
				if (tmpld != -1)
				{
					rampUpProcPriority (tmpld,-1);
				}			 
			}	
		}
	} 		

}

/* get max of p_i waiting on any of the locks held by process pid */
int getMaxWaitProcPrioForPI (int pid)
{
	int i;
	int maxprio = -1;
	struct pentry *pptr;
	struct lentry *lptr;

	pptr = &proctab[pid];
	
	for (i=0;i<NLOCKS;i++)
	{
		if (pptr->bm_locks[i] == 1)
		{
			lptr = &rw_locks[i];
			if (maxprio < lptr->lprio)
			{
				maxprio = lptr->lprio;
			}
		}		
	}
		
	return maxprio;
}

/* check and get the lock id for which the process pid is in wait queue for priority inheritance  */
int checkProcessTransitivityForPI (int pid)
{
	int ld;
	struct pentry *pptr;

	pptr = &proctab[pid];
	ld = pptr->wait_lockid;
	if (isbadlock(ld))
	{
		return -1;
	}
	else
	{
		return ld;
	}			
}	

int getProcessPriority(struct pentry *pptr)
{

	if (pptr->pinh == 0)
	{
		return pptr->pprio;
	}
	else
	{
		return pptr->pinh;
	}

}

