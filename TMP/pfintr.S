/*
 *	Modified by Rahul Sethi
 */
/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr	/* Interrupt Handler	*/
	   .globl  pferrcode 	/* Error Code		*/
pfintr:
	popl pferrcode
	pushfl
	cli
	pushal
	call pfint
	popal
	popfl
	iret

