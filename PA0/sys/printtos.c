#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>


static unsigned long *esp;
static unsigned long *ebp;
void printtos()

{
	int pid = getpid();
	struct pentry *proc = &proctab[pid];
	unsigned long *spbefore,*sp;
	kprintf("\n\nvoid printtos() ");
	if (pid != 0 && isbadpid(pid)) /* when there is some error */
	{
		kprintf("\nBad Process");
	}
	else
	{
		asm("movl %ebp,ebp");
		spbefore = ebp; /* top of stack just before call of function will be pointed by base pointer */
		kprintf("\n\nBefore[0x%08x]: 0x%08x",spbefore,*spbefore);
		
		asm("movl %esp,esp");
		sp = esp;
		kprintf("\nAfter[0x%08x]: 0x%08x",sp,*sp);
		/* 4 addresses and values below stack pointer */
		kprintf("\n	element[0x%08x]: 0x%08x",sp+1,*(sp+1));		
		kprintf("\n	element[0x%08x]: 0x%08x",sp+2,*(sp+2));
		kprintf("\n	element[0x%08x]: 0x%08x",sp+3,*(sp+3));
		kprintf("\n	element[0x%08x]: 0x%08x",sp+4,*(sp+4));	
	}
	

}
