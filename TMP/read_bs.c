/*
 *	Author: Rahul Sethi
 *	Last Modified: 11/19/2018
 */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {
	
	STATWORD ps;
	disable(ps);
	
	if(	(unsigned)dst < 0 || 
		(unsigned)dst >= (NBPG * NBPG) ||
		bs_id < 0 ||
		bs_id >= 8 ||
		page < 0 ||
		page > 256
	){
		restore(ps);
		return SYSERR;
	}

	void * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;

	bcopy(phy_addr, (void*)dst, NBPG);
	restore(ps);
	return OK;
}


