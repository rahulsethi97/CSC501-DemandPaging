/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>


extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */

unsigned top;

SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);

	struct mblock *curr , *next;
	
	if(size == 0){
		restore(ps);
		return SYSERR;
	}else if((4096*NBPG) > block){
		restore(ps);
		return SYSERR;
	}else{
		next = proctab[currpid].vmemlist;
		curr = proctab[currpid].vmemlist -> mnext;
		while(curr != NULL && curr < block){
			next = curr;
			curr = curr -> mnext;
		}
		next -> mnext = block;
		next = block;
		block -> mlen = size;
		block -> mnext = curr;


		restore(ps);
		return OK;
	}
	restore(ps);
	return SYSERR;
}
