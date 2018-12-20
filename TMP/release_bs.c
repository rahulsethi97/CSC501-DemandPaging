/*
 *	Author: Rahul Sethi
 *	Last Modified: 11/19/2018
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
	
	STATWORD ps;
        disable(ps);
	

	/* release the backing store with ID bs_id */
	if(bs_id < 0 || bs_id > 7){
                //Invalid bs_id or npages greater than 256
                restore(ps);
                return SYSERR;
	}else{
		bs_map_t *bptr = &bsm_tab[bs_id];
		bptr->bs_status = BSM_UNMAPPED;
		bptr->bs_pid = -1;
		bptr->bs_vpno = 4096;
		bptr->bs_npages = 0;
		bptr->bs_sem = 0;
		bptr->oneTimeFlag = 0;
		restore(ps);
		return OK;
	}
}

