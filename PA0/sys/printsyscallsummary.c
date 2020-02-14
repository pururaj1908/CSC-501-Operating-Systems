#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

struct processAccount procAccountingTab[1000];

int traceEnable;

void printsyscallsummary()
{
	kprintf("\n\nvoid printsyscallsummary()");
	int i = 1;
	int j = 0;
	int avgTime = 0;
	char* syscallMapping[27];
	syscallMapping[0] = "freemem";
	syscallMapping[1] = "chprio";
	syscallMapping[2] = "getpid";
        syscallMapping[3] = "getprio";
	syscallMapping[4] = "gettime";
        syscallMapping[5] = "kill";
	syscallMapping[6] = "receive";
        syscallMapping[7] = "recvclr";
	syscallMapping[8] = "recvtim";
        syscallMapping[9] = "resume";
	syscallMapping[10] = "scount";
        syscallMapping[11] = "sdelete";
	syscallMapping[12] = "send";
        syscallMapping[13] = "setdev";
	syscallMapping[14] = "setnok";
        syscallMapping[15] = "screate";
	syscallMapping[16] = "signal";
        syscallMapping[17] = "signaln";
	syscallMapping[18] = "sleep";
        syscallMapping[19] = "sleep10";
	syscallMapping[20] = "sleep100";
        syscallMapping[21] = "sleep1000";
	syscallMapping[22] = "sreset";
        syscallMapping[23] = "stacktrace";
	syscallMapping[24] = "suspend";
        syscallMapping[25] = "unsleep";
	syscallMapping[26] = "wait";

	for(i=1;i<=NPROC;i++)
	{
		if(procAccountingTab[i].dirtyFlag)
		{
			kprintf("\nProcess [pid:%d]",currpid);
			for(j=0;j<27;j++)
			{
				if(procAccountingTab[i].syscallCount[j]>0)
				{
					avgTime =(int) (procAccountingTab[i].sumExecTime[j] / procAccountingTab[i].syscallCount[j]);		
					kprintf("\nSyscall: sys_%s, count: %d, average execution time: %d (ms)",syscallMapping[j], procAccountingTab[i].syscallCount[j],avgTime);
				}
			}
		}	
	}
}


void syscallsummary_start()
{
	int i = 1;
	int j = 0;
	/* initialize procAccounting table for all the processes */
	for(i=1;i<=NPROC;i++)
	{
		procAccountingTab[i].dirtyFlag = 0;
		for(j=0;j<27;j++)
		{
			procAccountingTab[i].syscallCount[j] = 0;
			procAccountingTab[i].sumExecTime[j] = 0;
		}
	}
	traceEnable = 1;

}

void syscallsummary_stop()
{

	traceEnable = 0;
	
}

void calcSysCallAccounting(int sysid,unsigned long timeElapsed)
{
	if (currpid > 0 && currpid <=NPROC)
	{
	
        	procAccountingTab[currpid].syscallCount[sysid]++;
        	procAccountingTab[currpid].sumExecTime[sysid] = procAccountingTab[currpid].sumExecTime[sysid] + timeElapsed;
        	procAccountingTab[currpid].dirtyFlag = 1;
	}
}
