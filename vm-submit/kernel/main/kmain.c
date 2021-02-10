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
#include "kernel.h"
#include "errno.h"

#include "util/gdb.h"
#include "util/init.h"
#include "util/debug.h"
#include "util/string.h"
#include "util/printf.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/pagetable.h"
#include "mm/pframe.h"

#include "vm/vmmap.h"
#include "vm/shadowd.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "main/acpi.h"
#include "main/apic.h"
#include "main/interrupt.h"
#include "main/gdt.h"

#include "proc/sched.h"
#include "proc/proc.h"
#include "proc/kthread.h"

#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "drivers/disk/ata.h"
#include "drivers/tty/virtterm.h"
#include "drivers/pci.h"

#include "api/exec.h"
#include "api/syscall.h"

#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/fcntl.h"
#include "fs/stat.h"

#include "test/kshell/kshell.h"
#include "test/s5fs_test.h"

GDB_DEFINE_HOOK(boot)
GDB_DEFINE_HOOK(initialized)
GDB_DEFINE_HOOK(shutdown)

static void      *bootstrap(int arg1, void *arg2);
static void      *idleproc_run(int arg1, void *arg2);
static kthread_t *initproc_create(void);
static void      *initproc_run(int arg1, void *arg2);
static void       hard_shutdown(void);

static context_t bootstrap_context;
extern int gdb_wait;
extern void *faber_thread_test(int, void *);
extern void *sunghan_test(int, void *);
extern void *sunghan_deadlock_test(int, void *);

extern void *vfstest_main(int, void *);
extern int faber_fs_thread_test(kshell_t *ksh, int argc, char **argv);
extern int faber_directory_test(kshell_t *ksh, int argc, char **argv);

int COUNT = 0;

/**
 * This is the first real C function ever called. It performs a lot of
 * hardware-specific initialization, then creates a pseudo-context to
 * execute the bootstrap function in.
 */
void
kmain()
{
        GDB_CALL_HOOK(boot);

        dbg_init();
        dbg(DBG_PRINT, "Test num exec %d\n", ++COUNT);
        dbgq(DBG_CORE, "Kernel binary:\n");
        dbgq(DBG_CORE, "  text: 0x%p-0x%p\n", &kernel_start_text, &kernel_end_text);
        dbgq(DBG_CORE, "  data: 0x%p-0x%p\n", &kernel_start_data, &kernel_end_data);
        dbgq(DBG_CORE, "  bss:  0x%p-0x%p\n", &kernel_start_bss, &kernel_end_bss);

        page_init();

        pt_init();
        slab_init();
        pframe_init();

        acpi_init();
        apic_init();
        pci_init();
        intr_init();

        gdt_init();

        /* initialize slab allocators */
#ifdef __VM__
        anon_init();
        shadow_init();
#endif
        vmmap_init();
        proc_init();
        kthread_init();

#ifdef __DRIVERS__
        bytedev_init();
        blockdev_init();
#endif

        void *bstack = page_alloc();
        pagedir_t *bpdir = pt_get();
        KASSERT(NULL != bstack && "Ran out of memory while booting.");
        /* This little loop gives gdb a place to synch up with weenix.  In the
         * past the weenix command started qemu was started with -S which
         * allowed gdb to connect and start before the boot loader ran, but
         * since then a bug has appeared where breakpoints fail if gdb connects
         * before the boot loader runs.  See
         *
         * https://bugs.launchpad.net/qemu/+bug/526653
         *
         * This loop (along with an additional command in init.gdb setting
         * gdb_wait to 0) sticks weenix at a known place so gdb can join a
         * running weenix, set gdb_wait to zero  and catch the breakpoint in
         * bootstrap below.  See Config.mk for how to set GDBWAIT correctly.
         *
         * DANGER: if GDBWAIT != 0, and gdb is not running, this loop will never
         * exit and weenix will not run.  Make SURE the GDBWAIT is set the way
         * you expect.
         */
        while (gdb_wait) ;
        context_setup(&bootstrap_context, bootstrap, 0, NULL, bstack, PAGE_SIZE, bpdir);
        context_make_active(&bootstrap_context);

        panic("\nReturned to kmain()!!!\n");
}

/**
 * Clears all interrupts and halts, meaning that we will never run
 * again.
 */
static void
hard_shutdown()
{
#ifdef __DRIVERS__
        vt_print_shutdown();
#endif
        __asm__ volatile("cli; hlt");
}

/**
 * This function is called from kmain, however it is not running in a
 * thread context yet. It should create the idle process which will
 * start executing idleproc_run() in a real thread context.  To start
 * executing in the new process's context call context_make_active(),
 * passing in the appropriate context. This function should _NOT_
 * return.
 *
 * Note: Don't forget to set curproc and curthr appropriately.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
bootstrap(int arg1, void *arg2)
{
        /* If the next line is removed/altered in your submission, 20 points will be deducted. */
        dbgq(DBG_TEST,"SIGNATURE: 53616c7465645f5f63542fd3503b176f0906f15613bccb2985fd59d010ec3ca0920a8d2f3e920279df8792ab7db8bb65\n");
        pt_template_init();
        proc_t *idle_proc = proc_create("idle_proc");
        curproc = idle_proc;
        KASSERT(NULL != curproc);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");
        KASSERT(PID_IDLE == curproc->p_pid);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");
        kthread_t *idle_thread = kthread_create(idle_proc, idleproc_run, arg1, arg2);
        curthr = idle_thread;
        KASSERT(NULL != curthr);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");
        context_make_active(&(idle_thread->kt_ctx));
        panic("weenix returned to bootstrap()!!! BAD!!!\n");
        return NULL;
}

/**
 * Once we're inside of idleproc_run(), we are executing in the context of the
 * first process-- a real context, so we can finally begin running
 * meaningful code.
 *
 * This is the body of process 0. It should initialize all that we didn't
 * already initialize in kmain(), launch the init process (initproc_run),
 * wait for the init process to exit, then halt the machine.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
idleproc_run(int arg1, void *arg2)
{
        int status;
        pid_t child;

        /* create init proc */
        kthread_t *initthr = initproc_create();
        dbg(DBG_PRINT, "init thread created successfully\n");
        init_call_all();
        GDB_CALL_HOOK(initialized);

        /* Create other kernel threads (in order) */

#ifdef __VFS__
        /* Once you have VFS remember to set the current working directory
         * of the idle and init processes */
        // NOT_YET_IMPLEMENTED("VFS: idleproc_run");

        //struct vnode   *p_cwd; is the current working dir 
        //vfs_root_vn is root vnode
        // dbg(DBG_PRINT, "++++++++++++++0. current count is %d\n", vfs_root_vn->vn_refcount);
        dbg(DBG_PRINT, "(GRADING2A)\n");
        curproc->p_cwd=vfs_root_vn;
        //increment refcount
        vref(vfs_root_vn);
        
        //initthr->kt_proc is the thread's process
        //struct vnode   *p_cwd; is the current working dir 
        //vfs_root_vn is root vnode
        initthr->kt_proc->p_cwd=vfs_root_vn;
        //increment refcount
        vref(vfs_root_vn);
        // dbg(DBG_PRINT, "++++++++++++++1. current count is %d\n", vfs_root_vn->vn_refcount);

        /* Here you need to make the null, zero, and tty devices using mknod */
        /* You can't do this until you have VFS, check the include/drivers/dev.h
         * file for macros with the device ID's you will need to pass to mknod */
        // NOT_YET_IMPLEMENTED("VFS: idleproc_run");
        dbg(DBG_PRINT, "(GRADING2A)\n");
        do_mkdir("/dev");
        // dbg(DBG_PRINT, "++++++++++++++2. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "idle_proc 1(GRADING2B)\n");

        // struct stat buf;
        // int ret = do_stat("/dev", &buf);
        // dbg(DBG_PRINT, "check for /dev %d\n",ret);


        /* vnode vn_mode masks */

        // #define S_IFCHR         0x0100 /* character special */
        // #define S_IFDIR         0x0200 /* directory */
        // #define S_IFBLK         0x0400 /* block special */
        // #define S_IFREG         0x0800 /* regular */
        // #define S_IFLNK         0x1000 /* symlink */
        // #define S_IFIFO         0x2000 /* fifo/pipe */


        // #define MKDEVID(major, minor)   (((major) << MINOR_BITS) | (minor))

        /*
        * A Weenix "device identifier" is the concatenation of:
        *     - a "driver number" or "device type" (major number)
        *     - a "device number" (minor number)
        *
        * The device identifiers for block devices and character devices are
        * independent. That is, you could have both a block device and a char device
        * with major 3, minor 5 (for example). They would be distinct.
        *
        * Weenix's device number allocation/assignment scheme is as follows:
        *
        *     - major 0 (byte or block), minor 0: reserved as an analogue of NULL
        *       for device id's
        *
        *     - char major 1:         Memory devices (mem)
        *         - minor 0:          /dev/null       The null device
        *         - minor 1:          /dev/zero       The zero device
        *
        *     - char major 2:         TTY devices (tty)
        *         - minor 0:          /dev/tty0       First TTY device
        *         - minor 1:          /dev/tty1       Second TTY device
        *         - and so on...
        *
        *     - block major 1:        Disk devices
        *         - minor 0:          first disk device
        *         - minor 1:          second disk device
        *         - and so on...
        */

       // #define S_IFCHR         0x0100 /* character special */
       
        // - char major 1:         Memory devices (mem)
        // *         - minor 0:          /dev/null       The null device
        // *         - minor 1:          /dev/zero       The zero device
        do_mknod("/dev/null",S_IFCHR,MKDEVID(1,0));
        // dbg(DBG_PRINT, "++++++++++++++3. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "/dev/null\n");
        do_mknod("/dev/zero",S_IFCHR,MKDEVID(1,1));
        // dbg(DBG_PRINT, "++++++++++++++4. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "/dev/zero\n");

        // - char major 2:         TTY devices (tty)
        // *         - minor 0:          /dev/tty0       First TTY device
        // *         - minor 1:          /dev/tty1       Second TTY device
        // *         - and so on...
        do_mknod("/dev/tty0",S_IFCHR,MKDEVID(2,0));
        // dbg(DBG_PRINT, "++++++++++++++5. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "/dev/tty0\n");
        do_mknod("/dev/tty1",S_IFCHR,MKDEVID(2,1));
        // dbg(DBG_PRINT, "++++++++++++++6. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "/dev/tty1\n");

        // The device identifiers for block devices and character devices are
        // independent. That is, you could have both a block device and a char device
        // with major 3, minor 5 (for example). They would be distinct.
        
        // #define S_IFBLK         0x0400 /* block special */
        do_mknod("/dev/sda", S_IFBLK, MKDEVID(1,0));
        // dbg(DBG_PRINT, "++++++++++++++7. current count is %d\n", vfs_root_vn->vn_refcount);
        // dbg(DBG_PRINT, "/dev/sda\n");
        dbg(DBG_PRINT, "(GRADING2A)\n");

#endif

        /* Finally, enable interrupts (we want to make sure interrupts
         * are enabled AFTER all drivers are initialized) */
        intr_enable();

        /* Run initproc */
        sched_make_runnable(initthr);
        /* Now wait for it */
        child = do_waitpid(-1, 0, &status);
        KASSERT(PID_INIT == child);

#ifdef __MTP__
        kthread_reapd_shutdown();
#endif


#ifdef __SHADOWD__
        /* wait for shadowd to shutdown */
        shadowd_shutdown();
#endif

#ifdef __VFS__
        /* Shutdown the vfs: */
        dbg_print("weenix: vfs shutdown...\n");
        vput(curproc->p_cwd);
        if (vfs_shutdown())
                panic("vfs shutdown FAILED!!\n");

#endif

        /* Shutdown the pframe system */
#ifdef __S5FS__
        pframe_shutdown();
#endif

        dbg_print("\nweenix: halted cleanly!\n");
        GDB_CALL_HOOK(shutdown);
        hard_shutdown();
        return NULL;
}


/**
 * This function, called by the idle process (within 'idleproc_run'), creates the
 * process commonly refered to as the "init" process, which should have PID 1.
 *
 * The init process should contain a thread which begins execution in
 * initproc_run().
 *
 * @return a pointer to a newly created thread which will execute
 * initproc_run when it begins executing
 */
static kthread_t *
initproc_create(void)
{
        // NOT_YET_IMPLEMENTED("PROCS: initproc_create");
        proc_t *init_proc = proc_create("init_proc");
        KASSERT(NULL != init_proc);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");
        KASSERT(PID_INIT == init_proc->p_pid);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");
        kthread_t *init_thread = kthread_create(init_proc, initproc_run, 0, NULL);
        KASSERT(init_thread != NULL);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");
        return init_thread;
}

#ifdef __DRIVERS__
        int faberproc_kshell(kshell_t *kshell, int argc, char **argv)
        {
                int status = 0;
                KASSERT(kshell != NULL);
                proc_t *faber_test = proc_create("faber_test");
                kthread_t *faber_thread = kthread_create(faber_test, faber_thread_test, 0, 0);
                sched_make_runnable(faber_thread);
                do_waitpid(faber_test->p_pid, 0, &status);
                return 0;
        }

        int sunghan_kshell(kshell_t *kshell, int argc, char **argv)
        {
                int status = 0;
                KASSERT(kshell != NULL);
                proc_t *sunghan_test_proc = proc_create("sunghan_test");
                kthread_t *sunghan_test_thread = kthread_create(sunghan_test_proc, sunghan_test, 0, 0);
                sched_make_runnable(sunghan_test_thread);
                do_waitpid(sunghan_test_proc->p_pid, 0, &status);
                return 0;
        }

        int sunghan_deadlock_kshell(kshell_t *kshell, int argc, char **argv)
        {
                int status = 0;
                KASSERT(kshell != NULL);
                proc_t *sunghan_deadlock_test_proc = proc_create("sunghan_deadlock_test");
                kthread_t *sunghan_deadlock_test_thread = kthread_create(sunghan_deadlock_test_proc, sunghan_deadlock_test, 0, 0);
                sched_make_runnable(sunghan_deadlock_test_thread);
                do_waitpid(sunghan_deadlock_test_proc->p_pid, 0, &status);
                return 0;
        }

        static void *vfs_test(int arg1, void *arg2)
        {
                proc_t *proc = proc_create("vfs_test");

                kthread_t *thr = kthread_create(proc, (kthread_func_t)&vfstest_main, 1, NULL);

                sched_make_runnable(thr);
                int status;
                do_waitpid(proc->p_pid, 0, &status);

                return NULL;
        }
#endif

    /**
 * The init thread's function changes depending on how far along your Weenix is
 * developed. Before VM/FI, you'll probably just want to have this run whatever
 * tests you've written (possibly in a new process). After VM/FI, you'll just
 * exec "/sbin/init".
 *
 * Both arguments are unused.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
initproc_run(int arg1, void *arg2)
{
        //NOT_YET_IMPLEMENTED("PROCS: initproc_run");
        #ifdef __DRIVERS__
                // kshell_add_command("faberproc", faberproc_kshell, "Invoke faber tests");
                // kshell_add_command("sunghan_test", sunghan_kshell, "Invoke sunghan tests");
                // kshell_add_command("sunghan_deadlock_test", sunghan_deadlock_kshell, "Invoke sunghan deadlock tests");
                // kshell_add_command("vfstest", (kshell_cmd_func_t)&vfs_test, "Run vfs_test().");
                // kshell_add_command("thrtest", (kshell_cmd_func_t)&faber_fs_thread_test, "Run faber_fs_thread_test().");
                // kshell_add_command("dirtest", (kshell_cmd_func_t)&faber_directory_test, "Run faber_directory_test().");
                // int err = 0;
                // uint8_t ttyid = 0;
                // kshell_t *ksh = kshell_create(ttyid);
                // KASSERT(ksh && "did not create a kernel shell as expected");
                // while ((err = kshell_execute_next(ksh)) > 0)
                //         ;
                // KASSERT(err == 0 && "kernel shell exited with an error\n");
                // kshell_destroy(ksh);
                char *const argv[] = { "/sbin/init",NULL };
                char *const envp[] = { NULL };
                kernel_execve("/sbin/init", argv, envp);

        #endif
        return NULL;
}
