
	.text
	.globl zfunction
	.type zfunction, @function
zfunction:
	
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax /* load  value of param in eax */
			 /* displacement = 8 as we need to move two locations */ 
			 /* below after base pointer location first "return address" */ 
			 /* and then "paramter" can be pointed below it */	
	andl $0xFFFC01FF,%eax
	shr $8,%eax
	orl $0xFF000000,%eax
	
	movl %ebp,%esp
	popl %ebp
	ret 	 
