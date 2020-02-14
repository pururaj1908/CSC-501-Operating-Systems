/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;
	disable(ps);

	int bs_id;
	int pid;
	struct mblock *bs_badd;
	pid = create(procaddr,ssize,priority,name,nargs,args);
	
	/* sanity check to get backing store */
	if (get_bsm(&bs_id) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	
	/* add mapping to the bsm_tab table with requested heap size */
	bsm_map(pid,4096,bs_id,hsize);
	/* setting backing store private as it has been used to allocate private heap */
	bsm_tab[bs_id].bs_has_vheap = 1;	
	/* allocate heap size in corresponding backing store ID */
	bs_badd = BACKING_STORE_BASE + (bs_id * BACKING_STORE_UNIT_SIZE); 	
	bs_badd->mlen = hsize * NBPG;
	bs_badd->mnext = NULL;

	/* set vheap pages and vmemlist  */
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vmemlist->mnext = 4096 * NBPG;
	
	restore(ps);	
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
