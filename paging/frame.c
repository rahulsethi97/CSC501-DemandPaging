/* frame.c - manage physical frames */
/*
 * Modified by Rahul Sethi
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	int idx;
	STATWORD ps;
	disable(ps);
	for(idx = 0 ; idx < 3072 ; idx++){
		frm_tab[idx].fr_status = FRM_UNMAPPED;
		frm_tab[idx].fr_pid = -1;
		frm_tab[idx].fr_vpno = 4096;
		frm_tab[idx].fr_refcnt = 0;
		frm_tab[idx].fr_type = FR_PAGE;
		frm_tab[idx].fr_dirty = 0;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int idx;
	for(idx = 0 ; idx < 3072; idx++){
		if(frm_tab[idx].fr_status == FRM_UNMAPPED){
			*avail = idx;
			restore(ps);
			return OK;
		}
	}	
	if(grpolicy() == SC){
		idx = getFrame(1);
	}else if(grpolicy() == AGING){
		idx = getFrame(2);
	}
	free_frm(idx);
	*avail = idx;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int idx)
{
	STATWORD ps;
	disable(ps);
	if(idx < 0 || idx > 3072){
		restore(ps);
		return SYSERR;
	}
	
	unsigned long vAdd , pdbr;	
	unsigned int pTo , pDo;
	int pageTableFrame, store, fId, num;
	pd_t *pDe; 
	pt_t *pTe;

	if(frm_tab[idx].fr_type == FR_PAGE){
		fId = frm_tab[idx].fr_pid;
		pdbr = proctab[fId].pdbr;
		vAdd = frm_tab[idx].fr_vpno;

		pTo = vAdd & 1023;
		pDo = vAdd / 1024;		
		pDe = pdbr + (pDo * sizeof(pd_t));
		pTe = (pDe -> pd_base * NBPG) + (pTo * sizeof(pt_t));
		store = proctab[frm_tab[idx].fr_pid].store;
		num = frm_tab[idx].fr_vpno - proctab[fId].vhpno;

		write_bs( (idx+FRAME0) * NBPG , store, num);

		pTe->pt_pres = 0;
		pageTableFrame = pDe -> pd_base - FRAME0;
		
		frm_tab[pageTableFrame].fr_refcnt -= 1;
		
		if(frm_tab[pageTableFrame].fr_refcnt == 0){
			frm_tab[pageTableFrame].fr_pid = -1;
			frm_tab[pageTableFrame].fr_status = FRM_UNMAPPED;
			frm_tab[pageTableFrame].fr_type = FR_PAGE;
			frm_tab[pageTableFrame].fr_vpno = 4096;
			pDe->pd_pres = 0;
		}
		restore(ps);
		return OK;
	}
	restore(ps);
	return SYSERR;	

	return OK;
}



