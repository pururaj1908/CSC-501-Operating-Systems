/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
	   popl pferrcode /* store error code in global variable  */
	   pushfl /* save flag register  */
	   cli    /* clear all interrupts  */
	   pushal /* save general registers */
	   call pfint /* call page fault handler */
	   popal  /* restore general registers  */
	   popfl  /* restore flag register */
	   iret		
	   
	   					 	

