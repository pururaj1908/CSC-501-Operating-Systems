
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;

void printprocstks(int priority)
{

	struct pentry *proc;
	unsigned long *sp;
	int i = 1;
	kprintf("\n\nvoid printprocstks()");
	for(i = 1; i<= NPROC; i++)
	{
		proc = &proctab[i];
		if (proc != NULL && proc->pprio > priority)
		{
			if(i != currpid)
			{
				kprintf("\n\nProcess [%s]",proc->pname);
				kprintf("\n	pid: %d",i);
				kprintf("\n	priority: %d",proc->pprio);
				kprintf("\n	base: 0x%08x",proc->pbase);
				kprintf("\n	limit: 0x%08x",proc->plimit);
				kprintf("\n	len: %d",proc->pstklen);
				kprintf("\n	pointer: 0x%08x",proc->pesp);
			}
			else
			{
				kprintf("\n\nProcess [%s]",proc->pname);
                                kprintf("\n     pid: %d",currpid);
                                kprintf("\n     priority: %d",proc->pprio);
                                kprintf("\n     base: 0x%08x",proc->pbase);
                                kprintf("\n     limit: 0x%08x",proc->plimit);
                                kprintf("\n     len: %d",proc->pstklen);
				asm("movl %esp,esp");
				sp = esp;
                                kprintf("\n     pointer: 0x%08x",sp);
			}
		}
	}

}
