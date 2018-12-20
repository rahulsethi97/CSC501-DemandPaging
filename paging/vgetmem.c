/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>


extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;
	disable(ps);

	if(nbytes == 0 || nbytes > 256*NBPG){
		restore(ps);
		return ((WORD *)NULL);
	}else if((proctab[currpid].vmemlist -> mnext) == NULL){
		restore(ps);
                return ((WORD *)NULL);
	}

	struct mblock *curr , *next , *overFlow;

	nbytes = (unsigned int) roundmb(nbytes);
	curr = proctab[currpid].vmemlist -> mnext;
	next = proctab[currpid].vmemlist;

	while(curr != NULL){
		
		if( (curr -> mlen) == nbytes){
			next -> mnext = curr -> mnext;
			restore(ps);
			return ((WORD *)curr);
		}else if( (curr -> mlen) > nbytes){
			overFlow = (struct mblock *)((unsigned)curr + nbytes);
			next -> mnext = overFlow;
			overFlow -> mnext = curr -> mnext;
			overFlow -> mlen = (curr -> mlen) - nbytes;
			restore(ps);
			return ((WORD *)curr);
		}

		next = curr;
                curr = curr -> mnext;
	}

	restore(ps);
	return ((WORD *)NULL);
}


