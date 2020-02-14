/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>
/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
extern long zfunction(long);
int main()
{
	long x = 0;
	kprintf("\n\nHello World, Xinu lives\n\n");
	x  = zfunction(0xaabbccdd);
	kprintf("0x%08x",x);
	
	printsegaddress();
	printtos();
	printprocstks(1);
	syscallsummary_start();
	getpid();
	getpid();
	getprio(currpid);
	syscallsummary_stop();
	getpid();
	getpid();
	printsyscallsummary();
	return 0;
}
