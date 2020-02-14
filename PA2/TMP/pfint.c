/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  	STATWORD        ps;
        disable(ps);

	int i = 0;
	int j = 0;
	unsigned long vaddr;
	unsigned int vpd_offset;
	unsigned int vpt_offset;
	unsigned long pdbr;
	int new_frame;
	int new_frame_pt;
	int bs_id;
	int pageth;	
	virt_addr_t *virtual_add;
	pt_t *page_table_entry;
	pd_t *page_directory_entry;
	
	/* current process's pdbr location  */
	pdbr = proctab[currpid].pdbr;	

	/* virtual address on which page fault occured */
	vaddr = read_cr2();
	
	virtual_add = (virt_addr_t*)&vaddr;
	vpd_offset = virtual_add->pd_offset;
        vpt_offset = virtual_add->pt_offset;
		
        /* get corresponding page directory entry  */
 	page_directory_entry = pdbr + vpd_offset * sizeof(pd_t);
	
	/* page directory entry is not present */
	if(!page_directory_entry->pd_pres)
	{
		/* allocate frame for new page table */
		get_frm(&new_frame);
		page_table_entry = (pt_t*)((FRAME0 + new_frame) * NBPG);
		
		/* initializing 1024 page table entries for new page table  */
		for (i = 0; i < NFRAMES; i++)
		{
			
			page_table_entry[i].pt_pres = 0;
			page_table_entry[i].pt_write = 0;
			page_table_entry[i].pt_user = 0;
			page_table_entry[i].pt_pwt = 0;
			page_table_entry[i].pt_pcd = 0;
			page_table_entry[i].pt_acc = 0;
			page_table_entry[i].pt_dirty = 0;
			page_table_entry[i].pt_mbz = 0;
			page_table_entry[i].pt_global = 0;
			page_table_entry[i].pt_avail = 0;
			page_table_entry[i].pt_base = 0;
				
		}
		
		/* chagning the status and assigning type of new frame */
		frm_tab[new_frame].fr_status = FRM_MAPPED;
		frm_tab[new_frame].fr_type = FR_TBL;
		frm_tab[new_frame].fr_pid = currpid;
		
		/* set page directory entry to point to the above crated page table */
		
		page_directory_entry->pd_pres = 1;
		page_directory_entry->pd_write = 1;
		page_directory_entry->pd_user = 0;
		page_directory_entry->pd_pwt = 0;
		page_directory_entry->pd_pcd = 0;
		page_directory_entry->pd_acc = 0;
		page_directory_entry->pd_mbz = 0;
		page_directory_entry->pd_fmb = 0;
		page_directory_entry->pd_global = 0;
		page_directory_entry->pd_avail = 0;
		page_directory_entry->pd_base = FRAME0 + new_frame; /* address of newly created page table frame  */

	}
		
	/* allocate page table entry */
	page_table_entry = (pt_t*) (page_directory_entry->pd_base * NBPG + vpt_offset * sizeof(pt_t));				
		
	/* page table entry is not present */	
	if(!page_table_entry->pt_pres)
	{
		/* allocating new frame for the corresponding page table entry */
		get_frm(&new_frame_pt);

		/* append page replacement queue structure */
		append_pr_queue(new_frame_pt);
	
		/* updating the present and write bits and allocating base address with newly got frame */
		page_table_entry->pt_pres = 1;
		page_table_entry->pt_write = 1;
		page_table_entry->pt_base = FRAME0 + new_frame_pt;
	
		/* updating the status of newly created frame */
		frm_tab[new_frame_pt].fr_status = FRM_MAPPED;
		frm_tab[new_frame_pt].fr_type = FR_PAGE;
		frm_tab[new_frame_pt].fr_pid = currpid;
		frm_tab[new_frame_pt].fr_vpno = vaddr/NBPG;  /* Get 20 bit by right shifting 12 bits (4096) */


		//kprintf("\n vaddr :- %08x, vpd_off :- %d, vpt_offset :- %d, frame :- %d",vaddr,vpd_offset,vpt_offset,new_frame_pt);		
		/* increament reference count of page diretory entry frame as it has been referenced now */
		frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt++;
 
		/* get page from backing store corresponding to virtual address  */
		bsm_lookup(currpid,vaddr,&bs_id,&pageth);
		read_bs((char*)((FRAME0 + new_frame_pt) * NBPG),bs_id,pageth);
		
	
	}
	
	write_cr3(pdbr);	
	restore(ps);
	return OK;
}




