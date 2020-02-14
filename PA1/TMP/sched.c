#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>



int currSchedClass; 

void setschedclass (int sched_class)
{
	currSchedClass = sched_class;
}

int getschedclass()
{

	return currSchedClass;

}

void next_epoch()   /* to initialize next epoch for linux based scheduler  */
{
	int i;
	struct pentry *p;
	for (i = 0; i < NPROC; i++) 
	{
		p = &proctab[i];
		if (p->pstate != PRFREE)
		{
			if (p->counter == 0 || p->counter == p->timeQuantum) /*if no unused previous epoch quantum or process did not run at all*/ 
			{
				p->timeQuantum = p->pprio;
			} 
			else 
			{
				p->timeQuantum = p->pprio + (p->counter) / 2;
			}
			p->counter = p->timeQuantum;
			p->goodness =  p->pprio + p->counter;
		}
	}	
}
