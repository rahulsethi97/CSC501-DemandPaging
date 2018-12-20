/*
 *	Author: Rahul Sethi
 *	Last Modified: 11/19/2018
 */

/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[];
fr_map_t frm_tab[];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	int i;

   	for(i = 0; i < 8 ; i++)
   	{
      		bsm_tab[i].bs_status = BSM_UNMAPPED;
      		bsm_tab[i].bs_pid = 0;
      		bsm_tab[i].bs_vpno = 4096;
      		bsm_tab[i].bs_npages = 0;
      		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].oneTimeFlag = 0;
   	}
   	return OK;  	
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{	
	STATWORD ps;
	disable(ps);
	int i;

        for(i = 0; i < 8 ; i++)
        {
		if(bsm_tab[i].bs_status == BSM_UNMAPPED){
			*avail = i;
			restore(ps);
			return i;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
	disable(ps);

	if(i < 0 || i > 7){
		restore(ps);
		return SYSERR;	
	}

	bsm_tab[i].bs_status = BSM_UNMAPPED;
   	bsm_tab[i].bs_pid = -1;
   	bsm_tab[i].bs_vpno = 4096;
  	bsm_tab[i].bs_npages = 0;
   	bsm_tab[i].bs_sem = -1;
	bsm_tab[i].oneTimeFlag = 0;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	
	if(	pid <= 0 || 
		pid >= NPROC || 
		vaddr < 4096 ||
		store == NULL ||
		pageth == NULL){
		*store = -1;
		*pageth = -1;
		restore(ps);
		return SYSERR;

	}else{
		int i;
		for(i = 0 ; i < 8 ; i++){
			if(bsm_tab[i].bs_pid == pid){
				int a = (vaddr/NBPG) - bsm_tab[i].bs_vpno;
				if(a >= 0){
					*store = i;
					*pageth = (vaddr/NBPG) - bsm_tab[i].bs_vpno;
					restore(ps);
					return OK;
				}
			}
		}
	}

	*store = -1;
	*pageth = -1;
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int store, int npages)
{
	STATWORD ps;
	disable(ps);

	if(npages < 0 || npages > 256){
		restore(ps);
		return SYSERR;
	}else if(store < 0 || store > 7){
		restore(ps);
		return SYSERR;
	}else if(vpno < 4096){
		restore(ps);
		return SYSERR;
	}else{
		if(bsm_tab[store].bs_status == BSM_MAPPED){
			if(bsm_tab[store].oneTimeFlag == 0){
				proctab[pid].vhpno = vpno;
                        	proctab[pid].store = store;
			}else{
				restore(ps);
				return SYSERR;
			}
		}else if(bsm_tab[store].bs_status == BSM_UNMAPPED){
                        proctab[pid].vhpno = vpno;
			proctab[pid].store = store;
			bsm_tab[store].bs_status = BSM_MAPPED;
			bsm_tab[store].bs_npages = npages;
			bsm_tab[store].bs_pid = pid;
			bsm_tab[store].oneTimeFlag = 0;
			bsm_tab[store].bs_vpno = vpno;
			bsm_tab[store].bs_sem = 1;
		}else{
			restore(ps);
			return SYSERR;
		}
	}
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	disable(ps);

	if(vpno < 4096){
		restore(ps);
		return SYSERR;
	}
	int idx;
	int pageNo;
	if(bsm_lookup(pid , vpno*NBPG , &idx , &pageNo)){
		restore(ps);
		return SYSERR;
	}else{
		int i;
		unsigned long vAdd = vpno*NBPG;
		for(i = 0; i < NFRAMES; i++){
			if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE){
				bsm_lookup(pid,vAdd,&idx,&pageNo);
				write_bs( (i+NFRAMES)*NBPG, idx, pageNo);
			}
		}
		bsm_tab[idx].bs_status = BSM_UNMAPPED;
		bsm_tab[idx].bs_pid = -1;
		bsm_tab[idx].bs_npages = 0;
		bsm_tab[idx].oneTimeFlag = 0;
		bsm_tab[idx].bs_sem = 0;
		bsm_tab[idx].bs_vpno = 4096;
	

      }	
}

