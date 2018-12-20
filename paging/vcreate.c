

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL   newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
        int     *procaddr;              /* procedure address            */
        int     ssize;                  /* stack size in words          */
        int     hsize;                  /* virtual heap size in pages   */
        int     priority;               /* process priority > 0         */
        char    *name;                  /* name (for debugging)         */
        int     nargs;                  /* number of args that follow   */
        long    args;                   /* arguments (treated like an   */
                                        /* array in the code)           */
{
        STATWORD ps;
        disable(ps);
        int id = create(procaddr,ssize,priority,name,nargs,args);
        int bId;

        int res = get_bsm(&bId);        

        if(SYSERR == res){
                restore(ps);
                return SYSERR;
        }
        
        struct mblock *memoryBase;
        memoryBase = BACKING_STORE_BASE+(bId*BACKING_STORE_UNIT_SIZE);
        memoryBase->mlen = hsize*NBPG;
        memoryBase->mnext = NULL;

        bsm_tab[bId].bs_status = BSM_MAPPED;
        bsm_tab[bId].bs_vpno = 4096;
        bsm_tab[bId].bs_pid = id;
        bsm_tab[bId].bs_npages = hsize;
        bsm_tab[bId].bs_sem = 0;
        bsm_tab[bId].oneTimeFlag = 1;

        proctab[id].store = bId;
        proctab[id].vhpno = 4096;
        proctab[id].vhpnpages = hsize;
        proctab[id].vmemlist->mnext = 4096*NBPG;

        restore(ps);
        return id;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL   newpid()
{
        int     id;                    /* process id to return         */
        int     i;

        for (i=0 ; i<NPROC ; i++) {     /* check all NPROC slots        */
                if ( (id=nextproc--) <= 0)
                        nextproc = NPROC-1;
                if (proctab[id].pstate == PRFREE)
                        return(id);
        }
        return(SYSERR);
}

