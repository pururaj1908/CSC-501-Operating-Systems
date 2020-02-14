/* freemem.c - freemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 *  freemem  --  free a memory block, returning it to memlist
 *------------------------------------------------------------------------
 */
int traceEnable;
SYSCALL	freemem(struct mblock *block, unsigned size)
{
        unsigned long ctr1000;
        unsigned long timeElapsed = ctr1000;		

	STATWORD ps;    
	struct	mblock	*p, *q;
	unsigned top;

	if (size==0 || (unsigned)block>(unsigned)maxaddr
	    || ((unsigned)block)<((unsigned) &end))
	{
		if(traceEnable)
        	{
                	timeElapsed = ctr1000 - timeElapsed;
                	calcSysCallAccounting(0,timeElapsed);
       		}

		return(SYSERR);
	}
	size = (unsigned)roundmb(size);
	disable(ps);
	for( p=memlist.mnext,q= &memlist;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &memlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
		if(traceEnable)
        	{
                	timeElapsed = ctr1000 - timeElapsed;
                	calcSysCallAccounting(0,timeElapsed);
       		}

		return(SYSERR);
	}
	if ( q!= &memlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);

	if(traceEnable)
	{
		timeElapsed = ctr1000 - timeElapsed;
		calcSysCallAccounting(0,timeElapsed);
	}

	return(OK);
}
