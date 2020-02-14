/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>
#include <math.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	int expRand; /* randomn number for exponential distribution  */

	int next = q[rdyhead].qnext;	
		
	int proc = 0;	
	int max = 0;

	if(getschedclass() == EXPDISTSCHED)
	{

		expRand = (int) expdev(0.1);
		
		optr= &proctab[currpid];		
		
		if (next == NULLPROC && q[rdyhead].qnext != q[rdytail].qprev)
		{
			next = q[next].qnext;
		}
		else
		{
			if (next == NULLPROC)
                        {
                                if (optr->pstate == PRCURR)
                                {

                                        #ifdef RTCLOCK
                                                preempt = QUANTUM;
                                        #endif
                                        return(OK);
                                }
                                else   /* select Null process as there is no ready process in queue and next state of current process is not PRCURR  */
                                {

                                        nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
                                        nptr->pstate = PRCURR;
                                        #ifdef RTCLOCK
                                                preempt = QUANTUM;
                                        #endif

                                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                        return OK;
                                }
                        }
		}
		if(q[rdytail].qprev < NPROC) /* to check that there is at least one valid entry in ready queue */
		{
			while (q[next].qkey < expRand && q[next].qnext < NPROC)
			{
				next = q[next].qnext;
			}
		
			/* no switch needed as current process has higher priority 
 			 than random value and is lower than the lowest ready queue 
			 priority which is higher than random value */
			if ( optr->pstate == PRCURR && (
				(optr->pprio > expRand && ( q[next].qkey > optr->pprio || q[next].qkey <= expRand )) || 
				(optr->pprio < expRand && q[next].qkey < optr->pprio) ) ) {
				#ifdef RTCLOCK
					preempt = QUANTUM;
				#endif
				return(OK);	
			}
			/* force context switch */
		
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
                	        insert(currpid,rdyhead,optr->pprio);	
			}	
		
			/* remove selected lowest priority process greater than 
 	           	randomn value from queue  */
			nptr = &proctab[ (currpid = dequeue(next)) ];
			nptr->pstate = PRCURR;
			#ifdef RTCLOCK
				preempt = QUANTUM; /* same quantum used for round robin too to handle same priority case */
			#endif
		
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			return OK;		
		}
		else
		{
			#ifdef  RTCLOCK
		        	preempt = QUANTUM;
			#endif
			return(OK);
		}			
	}
	
	else if(getschedclass() == LINUXSCHED)
	{	
		
		optr = &proctab[currpid];
		
		optr->goodness += preempt - optr->counter;   /* dynamic priority*/
		optr->counter = preempt;
		
		if (preempt <= 0 || currpid == NULLPROC) /* if process used up its time quantum */
		{
			optr->counter = 0;
			optr->goodness = 0;
		}

		while (next != rdytail && next < NPROC) 
		{
			if (proctab[next].goodness > max) 
			{
				proc = next;
				max = proctab[next].goodness;
			}
			next = q[next].qnext;
		}
		
		/* next epoch as there is no runnable process having goodness value > 0 and next state of current process is not PRCURR */	
		if ((optr->pstate != PRCURR || optr->counter == 0) && max == 0)
		{
			next_epoch();  
			preempt = optr->counter;
			
			if (max == 0)
			{
				if (currpid == NULLPROC)
				{ return OK;}
				else
				{
					if (optr->pstate == PRCURR) /* next state is PRCURR so process wants to continue but counter = 0 */
					{
						optr->pstate = PRREADY;
						insert(currpid,rdyhead,optr->pprio);
					}
		
				 	nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
                                        nptr->pstate = PRCURR;
					
					#ifdef RTCLOCK
                                                preempt = QUANTUM;
                                        #endif

                                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                        return OK;			
				}	
			}				
		}
 
		/* no preemption of current process as its goodness value is highest */
		else if (optr->pstate == PRCURR && optr->goodness > 0 && optr->goodness > max)
		{
			preempt = optr->counter;
			return(OK);
		}

		else if ((optr->counter == 0 || optr->pstate != PRCURR || optr->goodness < max) && max > 0)
		{
			if (optr->pstate == PRCURR)
                        {
                        	optr->pstate = PRREADY;
                                insert(currpid,rdyhead,optr->pprio);
                        }

			nptr = &proctab[ (currpid = dequeue(proc)) ];
			nptr->pstate = PRCURR;

                        
                        preempt = nptr->counter;

                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                        return OK;	 	
		}

		return OK;
	}

	else // default scheduler
	{
		/* no switch needed if current process priority higher than next*/		
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) && 
			(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}	

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
                
                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return OK;	
	}
}
