/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 4096; /* starting VPN page  */
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_has_vheap = 0;				
	}
	
	restore(ps);
	return(OK);
			
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		if (bsm_tab[i].bs_status == BSM_UNMAPPED) /* return the first available unmapped backing store */
		{
			*avail = i;
			restore(ps);
			return(OK);
		}				
	}
			
	restore(ps);
	return SYSERR;
	
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD 	ps;
	disable(ps);
	
	if (i > -1 && i < 8 && bsm_tab[i].bs_status == BSM_MAPPED) /* if store id passed is valid and already mapped then unmap it */
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 4096; /* starting VPN page  */
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_has_vheap = 0;
		
		restore(ps);		
		return(OK);
	}	
	restore(ps);	
	return SYSERR;
	
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		if (bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == pid)
		{
			*store = i;  /* corresponding backing store ID for given pid */
			*pageth = (vaddr/NBPG) - bsm_tab[i].bs_vpno; /* the page offset within the store as vpno is starting page of mapped virtual memory range */ 
			restore(ps);
			return(OK);
		}				
	}
	
	restore(ps);
	return SYSERR;
	
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD 	ps;
	disable(ps);
	
	/* check if backing store has private heap or not and if it has then it should be only accessed by the process who posssess it */
	if (bsm_tab[source].bs_has_vheap == 0 || (bsm_tab[source].bs_has_vheap == 1 && bsm_tab[source].bs_pid == pid))
	{
		
		bsm_tab[source].bs_status = BSM_MAPPED;
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
		
		/* set corresponding mapping in proctab also */	
		proctab[currpid].vhpno = vpno;
		proctab[currpid].store = source;
	
		restore(ps);
		return(OK);
	}
	restore(ps);
	return SYSERR;					
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD 	ps;
	disable(ps);
	
	long vaddr = vpno * NBPG;
	int i = 0;
	int bs_id;
	int pageth;
 
	/* get the corresponding store ID for the virtual page number and process id  */
	bsm_lookup(pid,vpno,&bs_id,&pageth);
		
	/* clear the mapping of the corresponding backing store ID  */
	bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
	bsm_tab[bs_id].bs_pid = -1;
	bsm_tab[bs_id].bs_vpno = 4096;
	bsm_tab[bs_id].bs_npages = 0;
	bsm_tab[bs_id].bs_has_vheap = 0;
	
	/* write back the corresponding frame page to backing store as we are releasing the mapping to the process  */
	for (i = 0; i < NFRAMES; i++)
	{
		if (frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE)
		{
			write_bs((i + NFRAMES) * NBPG, bs_id, pageth );
			break;	
		}				
	}
	restore(ps);
	return(OK);
			
}


