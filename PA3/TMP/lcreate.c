
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate()
{
	STATWORD ps;    
	int	ld;

	disable(ps);
	if ((ld=newlock())==SYSERR) {
		restore(ps);
		return(SYSERR);
	}
	
	/* lqhead and lqtail were initialized at system startup */
	restore(ps);
	return(ld);
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	ld;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		ld=nextlock--;
		if (ld < 0)
			nextlock = NLOCKS-1;
		if (rw_locks[ld].lstate==LFREE) {
			rw_locks[ld].lstate = LUSED;
			return(ld);
		}
	}
	return(SYSERR);
}

