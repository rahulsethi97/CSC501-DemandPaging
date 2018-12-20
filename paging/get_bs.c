#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


bs_map_t bsm_tab[];

int get_bs(bsd_t bs_id, unsigned int npages) {
	STATWORD ps;
	disable(ps);
	
	if(bs_id < 0 || bs_id > 7 || npages > 256){
		//Invalid bs_id or npages greater than 256
		restore(ps);
		return SYSERR;
	}else if(bsm_tab[bs_id].oneTimeFlag == 1){
		restore(ps);
		return SYSERR;
	}else{
   	 	bs_map_t *bptr = &bsm_tab[bs_id];	
		if((bptr->bs_status) == BSM_MAPPED ){
			//BS already mapped
			if(bptr -> oneTimeFlag == 1 || bptr -> bs_sem == 1){
				restore(ps);
				return SYSERR;
			}else{
				bptr -> bs_pid = currpid;
				restore(ps);
				return bptr -> bs_npages;
			}
		}else{
        		bptr -> bs_pid = currpid;
        		bptr -> bs_status = BSM_MAPPED;
        		bptr -> bs_npages = npages;
			restore(ps);
			return bptr -> bs_npages;	
		}
	}

	restore(ps);
	return SYSERR;
}


