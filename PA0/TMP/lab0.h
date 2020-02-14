#ifndef _LAB0_H_
#define _LAB0_H_

struct processAccount
{
	int dirtyFlag; /* to check whether process has called atleast one sys call */	
	int syscallCount[27]; /* number of system calls are 27 */
	unsigned long sumExecTime[27];	

};

extern struct processAccount procAccountingTab[1000];
extern int traceEnable;

#endif
