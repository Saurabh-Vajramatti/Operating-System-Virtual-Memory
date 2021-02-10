/******************************************************************************/
/* Important Fall 2020 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
        /* Pointer argument and dummy return address, and userland dummy return
         * address */
        uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
        *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
        memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
        return esp;
}


// /*
//  * The implementation of fork(2). Once this works,
//  * you're practically home free. This is what the
//  * entirety of Weenix has been leading up to.
//  * Go forth and conquer.
//  */
int
do_fork(struct regs *regs)
{
        vmarea_t *vma, *clone_vma;
        pframe_t *pf;
        mmobj_t *to_delete, *new_shadowed;

        // NOT_YET_IMPLEMENTED("VM: do_fork");
        // return 0;


        /* preconditions check */

        /* the function argument must be non-NULL */
        KASSERT(regs != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
        /* the parent process, which is curproc, must be non-NULL */
        KASSERT(curproc != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
        /* the parent process must be in the running state and not in the zombie state */
        KASSERT(curproc->p_state == PROC_RUNNING);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        dbg(DBG_PRINT, "(GRADING3B)\n");

        /* Step1: Allocate a proc_t out of the procs structure using proc_create() */
        proc_t *forked_child_proc = proc_create("forked_child_proc");

        /* Step2: Copy the vmmap_t from the parent process into the child using vmmap_clone() */
        vmmap_t *forked_child_vmmap = vmmap_clone(curproc->p_vmmap);
        forked_child_proc->p_vmmap = forked_child_vmmap;
        forked_child_proc->p_vmmap->vmm_proc = forked_child_proc;
        forked_child_vmmap->vmm_proc = forked_child_proc;

        vma = NULL;

        list_t *vm_list = &forked_child_proc->p_vmmap->vmm_list;

        list_iterate_begin(vm_list, vma, vmarea_t, vma_plink)
        {
                vmarea_t *lookup_vmarea = vmmap_lookup(curproc->p_vmmap, vma->vma_start);
                int map_type_set = MAP_TYPE;
                int bitSet = vma->vma_flags & map_type_set;
                if ( bitSet != MAP_PRIVATE)
                {
                        /* Step3.2: For shared mappings, there is no need to copy the mmobj_t */
                        dbg(DBG_ANON, "(GRADING3B)\n");
                        vma->vma_obj = lookup_vmarea->vma_obj;
                        vma->vma_obj->mmo_ops->ref(vma->vma_obj);
                }
                else
                {
                        /* Step3.1: For each private mapping, point the vmarea_t at the new shadow object, which in turn should point to the original mmobj_t for the vmarea_t */
                        dbg(DBG_PRINT, "(GRADING3B)\n");

                        lookup_vmarea->vma_obj->mmo_ops->ref(lookup_vmarea->vma_obj);

                        mmobj_t *parent_shadow_obj = shadow_create();
                        mmobj_t *child_shadow_obj = shadow_create();

                        parent_shadow_obj->mmo_shadowed = lookup_vmarea->vma_obj;
                        child_shadow_obj->mmo_shadowed = lookup_vmarea->vma_obj;
                        struct mmobj* bottom_obj = lookup_vmarea->vma_obj->mmo_un.mmo_bottom_obj;
                        parent_shadow_obj->mmo_un.mmo_bottom_obj = bottom_obj;
                        child_shadow_obj->mmo_un.mmo_bottom_obj = bottom_obj;
                        
                        lookup_vmarea->vma_obj = parent_shadow_obj;
                        vma->vma_obj = child_shadow_obj;
                        
                }
                list_insert_tail(mmobj_bottom_vmas(lookup_vmarea->vma_obj), &vma->vma_olink);
        }
        list_iterate_end();

        /* Step4: Use kthread_clone() to copy the thread from the parent process into the child process */
        kthread_t *forked_child_thread = kthread_clone(curthr);
        forked_child_thread->kt_proc = forked_child_proc;

        /* middle conditions check */

        /* new child process starts in the running state */
        KASSERT(forked_child_proc->p_state == PROC_RUNNING);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        /* new child process must have a valid page table */
        KASSERT(forked_child_proc->p_pagedir != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        /* thread in the new child process must have a valid kernel stack */
        KASSERT(forked_child_thread->kt_kstack != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        /* adding forked thread to the list of forked child process' thread list */
        list_insert_tail(&forked_child_proc->p_threads, &forked_child_thread->kt_plink);
        

        /* Step5: Set up the new process thread context(kt_ctx)*/

        /* for child process */
        regs->r_eax = 0;

        /* c_pdptr - the page table pointer */
        forked_child_thread->kt_ctx.c_pdptr = forked_child_proc->p_pagedir;
        /* c_eip - function pointer for the userland_entry() function */
        forked_child_thread->kt_ctx.c_eip = (uint32_t) userland_entry;
        /* c_esp - the value returned by fork_setup_stack() */
        forked_child_thread->kt_ctx.c_esp = fork_setup_stack(regs,forked_child_thread->kt_kstack);
        /* c_kstack - the top of the new thread's kernel stack */
        forked_child_thread->kt_ctx.c_kstack = (uintptr_t) forked_child_thread->kt_kstack;
        /* c_kstacksz - size of the new thread's kernel stack */
        forked_child_thread->kt_ctx.c_kstacksz = DEFAULT_STACK_SIZE;
       

        /* Step6: Copy the file descriptor table of the parent into the child. Use fref() here */
        int i=0;
        while(1){
                if(i==NFILES){
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        break;
                }
                dbg(DBG_PRINT, "(GRADING3B)\n");
                forked_child_proc->p_files[i] = curproc->p_files[i];
                if(curproc->p_files[i]!=NULL){
                        dbg(DBG_PRINT, "(GRADING3B)\n");
                        fref(forked_child_proc->p_files[i]);
                }
                i++;
        }

        /* Step7: Set the child's working directory to point to the parent's working directory (once again, remember reference counts)  CHECK */

        /* Step8: Unmap the user land page table entries and flush the TLB */
        pt_unmap_range(curproc->p_pagedir, USER_MEM_LOW, USER_MEM_HIGH);
        tlb_flush_all();

        /* Step9: Set any other fields in the new process which need to be set */
        forked_child_proc->p_brk = curproc->p_brk;
        forked_child_proc->p_start_brk = curproc->p_start_brk;

        /* Step10: Make the new thread runnable */
        sched_make_runnable(forked_child_thread);

        /* for parent process */
        regs->r_eax = forked_child_proc->p_pid;
        return regs->r_eax;
}
