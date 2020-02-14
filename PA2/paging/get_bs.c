#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

    STATWORD 	ps;
    disable(ps);
		
  /* requests a new mapping of npages with ID map_id */
    if (bs_id > -1 && bs_id < 8 && npages >= 1 && npages <= 256 && bsm_tab[bs_id].bs_has_vheap == 0)
    {
	if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
	{
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_pid = currpid;
		
		restore(ps);
		return npages;		
	}
	else
	{
		restore(ps);
		return bsm_tab[bs_id].bs_npages;
	}	
    }
    
    restore(ps);
    return SYSERR;
   				
}


