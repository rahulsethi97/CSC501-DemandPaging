/*
 *      Author: Rahul Sethi
 *      Last Modified: 11/19/2018
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>

int write_bs(char *src, bsd_t bs_id, int page) {

	STATWORD ps;
        disable(ps);

        if(     (unsigned)src < 0 ||
                (unsigned)src >= (NBPG * NBPG) ||
                bs_id < 0 ||
                bs_id >= 8 ||
                page < 0 ||
                page > 256
        ){
                restore(ps);
                return SYSERR;
        }		

	char * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
	bcopy((void*)src, phy_addr, NBPG);
	restore(ps);
	return OK;
}

