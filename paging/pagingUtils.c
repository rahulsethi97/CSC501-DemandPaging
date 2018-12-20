#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


void removeFrame(int id)
{
	int idx = 0;
	for(idx = 0; idx < NFRAMES; idx++)
	{
		if(frm_tab[idx].fr_pid == id)
		{
		  	frm_tab[idx].fr_status = FRM_UNMAPPED;
		  	frm_tab[idx].fr_vpno = 4096;
		  	frm_tab[idx].fr_type = FR_PAGE;
		  	frm_tab[idx].fr_pid = -1;
		  	frm_tab[idx].fr_refcnt = 0;
		  	frm_tab[idx].fr_dirty = 0;
		}
	}
}

void initPageDir(int id){
	int frmNo = 0;
	int i;	
	pd_t *temp;

	int res = get_frm(&frmNo);
	if(res != SYSERR){
		proctab[id].pdbr = (FRAME0 + frmNo)*NBPG;
		frm_tab[frmNo].fr_pid = id;
		frm_tab[frmNo].fr_status = FRM_MAPPED;
		frm_tab[frmNo].fr_type = FR_DIR;
		temp = (pd_t*)((FRAME0+ frmNo)*NBPG);
		for(i = 0; i < NFRAMES; i++){
			temp[i].pd_write = 1;	
			if(i<4){
				temp[i].pd_pres = 1;
				temp[i].pd_base = FRAME0+i;
			}else{
				temp[i].pd_pres = 0;
			}	
		}
	}
}

int getFrame(int choice){
	int idx=0;
	unsigned long vAdd , pdbr;	
	unsigned int pTo , pDo;
	pd_t *pDe; 
	pt_t *pTe;
	int fId;

	if(choice == 1){
		
		while(1)
		{
			if(frm_tab[idx].fr_type == FR_PAGE){
				vAdd = frm_tab[idx].fr_vpno;
				pDo = vAdd / 1024;
				pTo = vAdd & 1023;
				fId = frm_tab[idx].fr_pid;
				pdbr = proctab[fId].pdbr;			
				pDe = pdbr + ( pDo * sizeof(pd_t) );
				pTe = (pDe -> pd_base * NBPG) + (pTo * sizeof(pt_t));
				if(pTe -> pt_acc==0){
					return idx;
				}
				else{
					pTe->pt_acc=0;
				}
			}
			idx++;
			idx = idx % NFRAMES;
		}

	}else if(choice == 2){
		
		int ans=-1;
		int age=300;
		int tempVal;
		
		while(idx<NFRAMES)
		{
			if(frm_tab[idx].fr_type==FR_PAGE){
				vAdd = frm_tab[idx].fr_vpno;
				fId = frm_tab[idx].fr_pid;
				pdbr = proctab[fId].pdbr;			
				pDo = vAdd/1024;
				pTo = vAdd&1023;
				pDe = pdbr+(pDo*sizeof(pd_t));
				pTe = (pDe->pd_base*NBPG)+(pTo*sizeof(pt_t));
				frm_tab[idx].age>>=1;

				if(pTe -> pt_acc == 1){
					tempVal = frm_tab[idx].age + 128;
					if(tempVal > 255)
						frm_tab[idx].age = 255;	
					else
						frm_tab[idx].age = tempVal;	
				}		

				if(frm_tab[idx].age < age){
					age = frm_tab[idx].age;
					ans = idx;
				}
			}
			idx++;
		}
		return ans;
	}
}

void customInit(){
	init_bsm();
  	init_frm();
  	int idx =  0, i , j;
  	pt_t *pg;

  	for(i = 0; i < 4; i++){

          	get_frm(&idx);
          	frm_tab[idx].fr_type = FR_TBL;
          	frm_tab[idx].fr_status = FRM_MAPPED;
          	frm_tab[idx].fr_pid = NULLPROC;
          	pg = (FRAME0+idx)*NBPG;
          	//pg = FRAME0*i;
		for(j = 0; j < 1024; j++){
            		pg->pt_pres = 1;
            		pg->pt_write = 1;
            		pg->pt_user = 0;
            		pg->pt_pwt = 0;
            		pg->pt_pcd = 0;
            		pg->pt_acc = 0;
            		pg->pt_dirty = 0;
            		pg->pt_mbz = 0;
            		pg->pt_global = 1;
            		pg->pt_avail = 0;
            		pg->pt_base = i*FRAME0+j;
            		pg++;
          	}
  	}

  	initPageDir(NULLPROC);
  	write_cr3(proctab[NULLPROC].pdbr);	
  	enable_paging();
}

void killUtil(int pid){
	release_bs(proctab[pid].store);
	removeFrame(pid);
}

int initPageTable(){
	int i , fNum;
	unsigned int frame_addr;
	int res = get_frm(&fNum);
	pt_t *pt;

	if(res != SYSERR){
		frame_addr = (FRAME0 + fNum) * NBPG;
		pt = (pt_t*)frame_addr;

		for(i=0 ; i < NFRAMES ; i++){
			pt[i].pt_write = 1;
			pt[i].pt_global = 0;
			pt[i].pt_acc = 0;
			pt[i].pt_mbz = 0;
			pt[i].pt_pcd = 0;
			pt[i].pt_avail = 0;
			pt[i].pt_base = 0;
			pt[i].pt_pres = 0;
			pt[i].pt_pwt = 0;
			pt[i].pt_dirty = 0;		
			pt[i].pt_user = 0;
		}
		return fNum;
	}
	return SYSERR;
}
