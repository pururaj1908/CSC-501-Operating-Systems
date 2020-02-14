/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int debug_mode;
extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
 	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	for (i = 0; i < NFRAMES; i++)
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0; /* initialize counter to 0  */
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;				
	}
	
	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{

  	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	int frame_id;
	for (i = 0; i < NFRAMES; i++)
	{
		if (frm_tab[i].fr_status == FRM_UNMAPPED) /* return the first available unmapped frame  */
		{
			*avail = i;	
			restore(ps);
			return(OK);
		}				
	}
		
	frame_id = pr_policy();
	
	/* sanity caheck */
	if (frame_id > -1)
	{
		free_frm(frame_id);
		*avail = frame_id;	
		restore(ps);
		return(OK);
	}	
		
	restore(ps);
	return SYSERR;
	 
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  	STATWORD 	ps;
	disable(ps);

	unsigned long vaddr;
	unsigned int vpd_offset;
	unsigned int vpt_offset;
	unsigned long pdbr;
	int bs_id;
	int pageth;
	
	pt_t *page_table_entry;
	pd_t *page_directory_entry;
	virt_addr_t *virtual_add;
	
	if(i >=0 && i < NFRAMES)
	{
		if(frm_tab[i].fr_type == FR_PAGE)
		{
			pdbr = proctab[frm_tab[i].fr_pid].pdbr;
			bs_id = proctab[frm_tab[i].fr_pid].store;
			pageth = frm_tab[i].fr_vpno - proctab[frm_tab[i].fr_pid].vhpno;

			vaddr = frm_tab[i].fr_vpno * NBPG;  /* Make full 32 bit by left shifting 12 bits (4096) */
	
			virtual_add = (virt_addr_t*)&vaddr;
			vpd_offset = virtual_add->pd_offset;
        		vpt_offset = virtual_add->pt_offset;
	
			//kprintf("\n pd offset %d, pt offset %d",vpd_offset,vpt_offset);
			
			page_directory_entry = pdbr + vpd_offset * sizeof(pd_t);
			page_table_entry = (pt_t*) (page_directory_entry->pd_base * NBPG + vpt_offset * sizeof(pt_t));

			write_bs((i + FRAME0) * NBPG,bs_id,pageth);
			
			/* reset present bit of page table entry  */
			page_table_entry->pt_pres = 0;
			
			/* decreament the reference count of corresponding page table frame */
			frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt--;

			/* if now reference count of corresponding page table frame becomes zero then unmap it */
			if (frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt == 0)
			{
				page_directory_entry->pd_pres = 0;
	
				frm_tab[page_directory_entry->pd_base - FRAME0].fr_status = FRM_UNMAPPED;
				frm_tab[page_directory_entry->pd_base - FRAME0].fr_type = FR_PAGE;
				frm_tab[page_directory_entry->pd_base - FRAME0].fr_pid = -1;
				frm_tab[page_directory_entry->pd_base - FRAME0].fr_vpno = 4096;	
			} 	
			
			restore(ps);
			return(OK);			   					
		}		
	}	
  
	restore(ps);
	return SYSERR;
}

/* initialize frame queue for page replacement */
void init_pr_queue()
{
	int i = 0;
	for (i = 0; i < NFRAMES; i++)
	{
		pr_qtab[i].fr_id = i;
		pr_qtab[i].fr_age = 0;
		pr_qtab[i].next = -1;
	}
}

void append_pr_queue(int fr_id)
{
	STATWORD 	ps;
	disable(ps);
	
	int current;
	int next;
	
	/* if frame queue is empty */
	if(pr_qhead == -1)
	{
		pr_qhead = fr_id;
		
		restore(ps);
		return(OK);
	}
	
	/* append the frame at the end of the queue  */
	else
	{
		next = pr_qtab[pr_qhead].next;
		current = pr_qhead;
		
		while (next != -1)
		{
			current = next;
			next = pr_qtab[next].next; 
		}
		
		pr_qtab[current].next = fr_id;
		pr_qtab[fr_id].next = -1;
	}			

	restore(ps);
	return(OK);	

}
/* page replacement policy  */
int pr_policy()
{
	STATWORD 	ps;
	disable(ps);
	
	unsigned long vaddr;
	unsigned int vpd_offset;
	unsigned int vpt_offset;
	unsigned long pdbr;
	int pageth;
	
	int current = pr_qhead; /* current starts with the head of the queue */
	int next = -1;	       /* initialize next frame to not point to any valid frame */	
	int prev = -1;	       /* initialize prev frame to not point to any valid frame	*/
	int fr_id = 0;         			
	int tmpprev;
	pt_t *page_table_entry;
	pd_t *page_directory_entry;
	virt_addr_t *virtual_add;
	
	// SC policy
	if(page_replace_policy == SC)
	{
		while(current != -1)
		{
			fr_id = current; /* initialize with current frame queue head */

			/* current process's pdbr location  */
	        	pdbr = proctab[currpid].pdbr;

			vaddr = frm_tab[current].fr_vpno * NBPG;  /* Make full 32 bit by left shifting 12 bits (4096) */

			virtual_add = (virt_addr_t*)&vaddr;
			vpd_offset = virtual_add->pd_offset;
        		vpt_offset = virtual_add->pt_offset;
				
			page_directory_entry = pdbr + vpd_offset * sizeof(pd_t);
			page_table_entry = (pt_t*) (page_directory_entry->pd_base * NBPG + vpt_offset * sizeof(pt_t));
			
			/* if referenced bit is set  */
			if (page_table_entry->pt_acc == 1)
			{
				/* as reference bit is already set we reset it  */
				page_table_entry->pt_acc = 0;

			}

			else if (page_table_entry->pt_acc == 0) 
			{
				/* head of the queue has to be removed  */
				if (prev == -1)
				{
					pr_qhead = pr_qtab[current].next;
					pr_qtab[current].next = -1;
	
					if (debug_mode == 1)
					{
						kprintf("\n\n Frame to be removed-  %d",fr_id);
					}

					restore(ps);
					return fr_id;			
				}
				
				/* in between frame to be removed  */	
				else
				{
					pr_qtab[prev].next = pr_qtab[current].next;
					pr_qtab[current].next = -1;

					if (debug_mode == 1)
					{
						kprintf("\n\n Frame to be removed-  %d",fr_id);
					}

					restore(ps);
					return fr_id;			
	
				}
			}
			prev = current;
			current = pr_qtab[current].next; 						
		}

		/* if no frame got thus all have been now set to acc = 0 and remove qhead thus this will be treated as circular */
		current = pr_qhead;
		pr_qhead = pr_qtab[pr_qhead].next;		
		pr_qtab[current].next = -1;

		if (debug_mode == 1)
		{
			kprintf("\n\n Frame to be removed-  %d",fr_id);
		}		
		restore(ps);
		return fr_id;						
	}
	
	
	// Aging policy
	else
	{
		fr_id = pr_qhead;
		while(current != -1)
		{
			
			/* decrease age by 2 by right shifting age with 1 bit  */	
			pr_qtab[current].fr_age = pr_qtab[current].fr_age >> 1; 	
		
			/* current process's pdbr location  */
	        	pdbr = proctab[currpid].pdbr;

			vaddr = frm_tab[current].fr_vpno * NBPG; /* Make full 32 bit by left shifting 12 bits (4096) */

			virtual_add = (virt_addr_t*)&vaddr;
			vpd_offset = virtual_add->pd_offset;
        		vpt_offset = virtual_add->pt_offset;
			
			//kprintf("\n in Aging vaddr %08x, vpd_offset %d, vpn_offset %d current %d",vaddr,vpd_offset,vpt_offset,current);
			
			page_directory_entry = pdbr + vpd_offset * sizeof(pd_t);
			page_table_entry = (pt_t*) (page_directory_entry->pd_base * NBPG + vpt_offset * sizeof(pt_t));
			
			/* if referenced bit is set  */
			if (page_table_entry->pt_acc == 1)
			{
				/* if accessed then increase age by 128 with maximum value possible 255*/
				if ((pr_qtab[current].fr_age + 128) > 255)
				{
					pr_qtab[current].fr_age = 255;
				}
				else
				{
					pr_qtab[current].fr_age += 128;
				} 		
			}

			if (pr_qtab[current].fr_age < pr_qtab[fr_id].fr_age)
			{
				tmpprev = prev;
				fr_id = current;
			}
			prev = current;	
			current = pr_qtab[current].next; 						
		}
		
		if (fr_id == pr_qhead)
		{
			current = pr_qhead;
			pr_qhead = pr_qtab[pr_qhead].next;		
			pr_qtab[current].next = -1;

		}
		else
		{
			pr_qtab[tmpprev].next = pr_qtab[fr_id].next;
			pr_qtab[fr_id].next = -1;
		}	 

		if (debug_mode == 1)
		{
			kprintf("\n\n Frame to be removed-  %d",fr_id);
		}		
		restore(ps);
		return fr_id;			
	}				
}

