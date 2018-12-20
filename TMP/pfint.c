/*
 *	Author: Rahul Sethi
 *	Last Modified: 11/19/2018
 */

/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

#define ul unsigned long
#define ui unsigned int


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	
	ul rVa , pdbr;
	virt_addr_t *vAs;  
	ui pgOffset,ptOffset , pdOffset;
	pd_t *pDe;
	pt_t *pTe; 
	int fId1 , fId2 , bS , pOs; 
	
	disable(ps);

	rVa = read_cr2();
	vAs = (virt_addr_t*)&rVa;

	pgOffset = vAs -> pg_offset;
	ptOffset = vAs -> pt_offset;
	pdOffset = vAs -> pd_offset;

	pdbr = proctab[currpid].pdbr;
	pDe = pdbr + pdOffset * sizeof(pd_t);

	if(pDe -> pd_pres ==0) //Not present
	{
		fId1 = initPageTable();

		pDe->pd_pres = 1;
		pDe->pd_write = 1;
		pDe->pd_pcd = 0;
		pDe->pd_acc = 0;
		pDe->pd_global = 0;
		pDe->pd_pwt = 0;
		pDe->pd_mbz = 0;
		pDe->pd_user = 1;
		pDe->pd_fmb = 0;
		pDe->pd_avail = 0;
		pDe->pd_base = fId1+FRAME0;

		frm_tab[fId1].fr_type = FR_TBL;
		frm_tab[fId1].fr_status = FRM_MAPPED;
		frm_tab[fId1].fr_pid = currpid;
				
	}

	pTe = (pt_t*)(pDe -> pd_base * NBPG + ptOffset * sizeof(pt_t));

	if(pTe -> pt_pres == 0)
	{
		get_frm(&fId2);

		pTe->pt_pres = 1;
		pTe->pt_write = 1;
		pTe->pt_user = 1;
		pTe->pt_base = (FRAME0+fId2);

		frm_tab[fId2].fr_status = FRM_MAPPED;
		frm_tab[fId2].fr_pid = currpid;
		frm_tab[fId2].fr_vpno = rVa/NBPG;
		frm_tab[fId2].fr_type = FR_PAGE;
		frm_tab[fId2].fr_dirty=0;

		frm_tab[pDe->pd_base-FRAME0].fr_refcnt++;
		
		bsm_lookup(currpid,rVa,&bS,&pOs);
		read_bs((char*)((FRAME0+fId2)*NBPG),bS,pOs);
	}

	write_cr3(pdbr);
	restore(ps);
	return OK;	
}


