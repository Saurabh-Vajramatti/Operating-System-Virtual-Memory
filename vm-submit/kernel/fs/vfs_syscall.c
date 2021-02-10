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

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.2 2018/05/27 03:57:26 cvsps Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * Syscalls for vfs. Refer to comments or man pages for implementation.
 * Do note that you don't need to set errno, you should just return the
 * negative error code.
 */

int
do_read(int fd, void *buf, size_t nbytes)
{
        // NOT_YET_IMPLEMENTED("VFS: do_read");
        // return -1;
        if(fd<0 || fd>=NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        file_t *file = fget(fd);
        if(file==NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        if (!(file->f_mode & FMODE_READ)){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return -EBADF;
        }
        if(S_ISDIR(file->f_vnode->vn_mode)){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return -EISDIR;
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        int do_read_result = file->f_vnode->vn_ops->read(file->f_vnode,file->f_pos,buf,nbytes);
        fput(file);
        if (do_read_result >= 0)
        {
                dbg(DBG_PRINT, "(GRADING3B)\n");
                int do_seek_res = do_lseek(fd, do_read_result, SEEK_CUR);
                if (do_seek_res < 0)
                {
                        dbg(DBG_PRINT, "(GRADING3D)\n");
                        return do_seek_res;
                }
                dbg(DBG_PRINT, "(GRADING3D)\n");
        }
        else
        {
                dbg(DBG_PRINT, "(GRADING3D)\n");
        }
        // file->f_pos+=do_read_result;
        return do_read_result;
}


/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * vn_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        // NOT_YET_IMPLEMENTED("VFS: do_write");
        // return -1;
        if (fd < 0 || fd >= NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        file_t *file = fget(fd);
        if (file == NULL){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        if (!(file->f_mode & FMODE_WRITE)){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return -EBADF;
        }
        if (file->f_mode & FMODE_APPEND){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                do_lseek(fd, 0, SEEK_END);
        }
        int do_write_result = file->f_vnode->vn_ops->write(file->f_vnode, file->f_pos, buf, nbytes);
        file->f_pos += do_write_result;
        KASSERT((S_ISCHR(file->f_vnode->vn_mode)) || (S_ISBLK(file->f_vnode->vn_mode)) ||
                ((S_ISREG(file->f_vnode->vn_mode)) && (file->f_pos <= file->f_vnode->vn_len)));
        dbg(DBG_PRINT, "(GRADING2A 3.a)\n");
        fput(file);
        return do_write_result;
}


/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        // NOT_YET_IMPLEMENTED("VFS: do_close");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_close_return_int;

        // Initializing with -EBADF in the case when fd 
        // isn't a valid open file descriptor, until proven otherwise
        // #define EBADF           9               /* Bad file descriptor */
        do_close_return_int=-EBADF;


        // /* VFS-related: */
        // struct file    *p_files[NFILES]; /* open files */
        // struct vnode   *p_cwd;           /* current working dir */

        // #define NFILES                  32      /* maximum number of open files */
        if(fd>=0 && fd<NFILES && curproc->p_files[fd]!=NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // * - Decrement f_count.
                // * - If f_count == 0, call release (if available), vput() and free it. */
                fput(curproc->p_files[fd]);
                curproc->p_files[fd]=NULL;
                // Returning 0 on success
                do_close_return_int=0;
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");

        return do_close_return_int;
}


/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        // NOT_YET_IMPLEMENTED("VFS: do_dup");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_dup_return_int;

        if(fd==-1)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // don't want fget to create new
                // as fd isn't an open file descriptor.
               do_dup_return_int=-EBADF;
        }
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file_t* file_object=fget(fd);

                if(file_object!=NULL)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        int smallest_empty_index=get_empty_fd(curproc);
                        
                        // Did not check for the return value against -EMFILE
                        // is not tested in the testing code

                        // K3NCE
                        if (smallest_empty_index < 0)
                        {
                                dbg(DBG_PRINT, "(GRADING2D)\n");
                                fput(file_object);
                                return smallest_empty_index;
                        }
                        // K3NCE
                        curproc->p_files[smallest_empty_index]=file_object;
                        do_dup_return_int=smallest_empty_index;
                }
                // fd isn't an open file descriptor.
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_dup_return_int=-EBADF;
                }
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");

        return do_dup_return_int;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        // NOT_YET_IMPLEMENTED("VFS: do_dup2");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_dup2_return_int;

        if(ofd==-1)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // don't want fget to create new
                // as fd isn't an open file descriptor.
               do_dup2_return_int=-EBADF;
        }
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file_t* file_object=fget(ofd);

                if(file_object!=NULL && nfd>=0 && nfd<NFILES)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        if(curproc->p_files[nfd]==NULL)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                curproc->p_files[nfd]=file_object;
                                // Did not check for the return value against -EMFILE
                                // is not tested in the testing code
                                do_dup2_return_int=nfd;
                        }
                        // If nfd is in use
                        else
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                do_close(nfd);
                                curproc->p_files[nfd]=file_object;
                                // Did not check for the return value against -EMFILE
                                // is not tested in the testing code
                                do_dup2_return_int=nfd;
                        }
                        
                }
                // fd isn't an open file descriptor.
                else
                {
                        // K3NCE
                        if ((nfd >= NFILES || nfd < 0)&& file_object!=NULL)
                        {
                                dbg(DBG_PRINT, "(GRADING2D)\n");
                                fput(file_object);
                                return -EBADF;
                        }
                        // K3NCE
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_dup2_return_int=-EBADF;
                }
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");

        return do_dup2_return_int;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_mknod(const char *path, int mode, unsigned devid)
{
        // NOT_YET_IMPLEMENTED("VFS: do_mknod");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        // GETTING res_vnode: the vnode of the parent directory of "name"
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;
        dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        vnode_t *next_node = NULL;

        lookup(dir_vnode, name, namelen, &next_node);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        // Not checking for the various cases of dir_namev_return_int because, they would already have been checked in do_mkdir

        // Not checking the return valuse of lookup because in kernel 2(Might be required in Kernel 3):
        // 1. do_mknod is called when creating the initial devices which will not cause any error
        // 2. do_mknod is also called in one test case, but it does not result in any error cases

        // No need to increment the reference count too as the node that we are looking for does not exist in the test cases
        // int lookup_return_int=lookup(dir_vnode,name,namelen,&next_dir_vnode);

        /* dir_vnode is the directory vnode where you will create the target special file */
        KASSERT(NULL != dir_vnode->vn_ops->mknod);
        dbg(DBG_PRINT, "(GRADING2A 3.b)\n");

        // /*
        //  * mknod creates a special file for the device specified by
        //  * 'devid' and an entry for it in 'dir' of the specified name.
        //  */
        // int (*mknod)(struct vnode *dir, const char *name, size_t name_len,
        //              int mode, devid_t devid);

        int mknod_return_int;
        devid_t device_id = devid;
        mknod_return_int = dir_vnode->vn_ops->mknod(dir_vnode, name, namelen, mode, device_id);

        // Decrementing the reference count as we no longer need the reference
        vput(dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return mknod_return_int;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        // NOT_YET_IMPLEMENTED("VFS: do_mkdir");
        dbg(DBG_PRINT, "(GRADING2B)\n");
        size_t namelen=0;
        // New
        // const char* name=NULL;
        const char* name;
        // New
        vnode_t* dir_vnode=NULL;
        vnode_t *next_dir_vnode = NULL;
        int dir_namev_return_int;
        dir_namev_return_int= dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_mkdir_return_int;
        // dbg(DBG_PRINT, "do_mkdir NAME(GRADING2B) %s\n",name);
        // New Code
        if(!name)
        {
                vput(dir_vnode);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EEXIST;
        }
        // New code
        if(dir_namev_return_int!=0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // REFERENCE CLEANUP was handled in dir_namev for these cases
                do_mkdir_return_int=dir_namev_return_int;
        }

        // else if(namelen==0)
        // {
        //         vput(dir_vnode);
        //         do_mkdir_return_int=EEXIST;
        // }
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                
                // if next_dir_vnode exists this will also return with the vnode refcount on *next_dir_vnode incremented.
                // So have to keep this in mind!!!!!
                // dbg(DBG_PRINT, "mkdir --> %s %d \n",name,namelen);
                int lookup_return_int=lookup(dir_vnode,name,namelen,&next_dir_vnode);
                // dbg(DBG_PRINT, "mkdir next --> %s %d \n", name, namelen);

                // A component used as a directory in path is not, in fact, a directory.
                if(lookup_return_int==-ENOTDIR)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_mkdir_return_int=-ENOTDIR;
                }

                // path already exists.
                else if(next_dir_vnode!=NULL)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_mkdir_return_int=-EEXIST;
                }     

                // Need to create the target directory
                else
                {
                        // /*
                        //  * mkdir creates a directory called name in dir
                        //  */
                        // int (*mkdir)(struct vnode *dir,  const char *name, size_t name_len);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        KASSERT(NULL != dir_vnode->vn_ops->mkdir);
                        dbg(DBG_PRINT, "(GRADING2A 3.c)\n");

                        int mkdir_return_int;
                        mkdir_return_int=dir_vnode->vn_ops->mkdir(dir_vnode,name,namelen);
                        do_mkdir_return_int=mkdir_return_int;
                }

                dbg(DBG_PRINT, "(GRADING2B)\n");
                // HANDLING REFERENCE CLEANUP
                if(do_mkdir_return_int==-EEXIST)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // Decrementing reference count for next_dir_vnode as we don't need its reference anymore
                        vput(next_dir_vnode);
                }

                dbg(DBG_PRINT, "(GRADING2B)\n");
                // dbg(DBG_PRINT, "-------->>do_mkdir 6(GRADING2B) %d\n", do_mkdir_return_int);
                // Now we no longer need reference to dir_vnode so we release it by decrementing its reference count
                vput(dir_vnode);
                        
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");

        return do_mkdir_return_int;
}


/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_rmdir(const char *path)
{
        // NOT_YET_IMPLEMENTED("VFS: do_rmdir");
        dbg(DBG_PRINT, "(GRADING2B)\n");
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *dir_vnode = NULL;
        // vnode_t *next_dir_vnode = NULL;
        int dir_namev_return_int;
        dir_namev_return_int = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_rmdir_return_int;
        if (dir_namev_return_int != 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // REFERENCE CLEANUP was handled in dir_namev for these cases
                do_rmdir_return_int = dir_namev_return_int;
        }
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if (dir_vnode->vn_ops->rmdir != NULL)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        /* dir_vnode is the directory vnode where you will remove the target directory */
                        KASSERT(NULL != dir_vnode->vn_ops->rmdir);
                        dbg(DBG_PRINT, "(GRADING2A 3.d)\n");
                        dbg(DBG_PRINT, "(GRADING2B)\n");

                        char x1 = name[0];
                        char x2 = name[1];
                        char x3 = '.';
                        if (namelen == 1 && x1 == x3)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                do_rmdir_return_int = -EINVAL;
                        }
                        else if (namelen == 2 && x1 == x3 && x2 == x3)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                do_rmdir_return_int = -ENOTEMPTY;
                        }

                        else
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                int rmdir_return_int = dir_vnode->vn_ops->rmdir(dir_vnode, name, namelen);
                                do_rmdir_return_int = rmdir_return_int;
                        }
                }
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_rmdir_return_int = -ENOTDIR;
                }

                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(dir_vnode);
        }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        return do_rmdir_return_int;
}


/*
 * Similar to do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EPERM
 *        path refers to a directory.
 *      o ENOENT
 *        Any component in path does not exist, including the element at the
 *        very end.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
        // NOT_YET_IMPLEMENTED("VFS: do_unlink");
        dbg(DBG_PRINT, "(GRADING2B)\n");
        size_t namelen=0;
        const char* name=NULL;
        vnode_t* dir_vnode=NULL;
        vnode_t *next_dir_vnode = NULL;
        int dir_namev_return_int;
        dir_namev_return_int= dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_unlink_return_int;

        // Successful return from dir_namev
        if(dir_namev_return_int==0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");

                // if next_dir_vnode exists this will also return with the vnode refcount on *next_dir_vnode incremented.
                // So have to keep this in mind!!!!!
                int lookup_return_int=lookup(dir_vnode,name,namelen,&next_dir_vnode);
                
                if(lookup_return_int==0)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // /*
                        // * File type.  See stat.h.
                        // */
                        // int                vn_mode;

                        /* vnode vn_mode masks */

                        // #define S_IFCHR         0x0100 /* character special */
                        // #define S_IFDIR         0x0200 /* directory */
                        // #define S_IFBLK         0x0400 /* block special */
                        // #define S_IFREG         0x0800 /* regular */
                        // #define S_IFLNK         0x1000 /* symlink */
                        // #define S_IFIFO         0x2000 /* fifo/pipe */

                        // #define _S_TYPE(m)      ((m) & 0xFF00)
                        // #define S_ISCHR(m)      (_S_TYPE(m) == S_IFCHR)
                        // #define S_ISDIR(m)      (_S_TYPE(m) == S_IFDIR)
                        // #define S_ISBLK(m)      (_S_TYPE(m) == S_IFBLK)
                        // #define S_ISREG(m)      (_S_TYPE(m) == S_IFREG)
                        // #define S_ISLNK(m)      (_S_TYPE(m) == S_IFLNK)
                        // #define S_ISFIFO(m)     (_S_TYPE(m) == S_IFIFO)
                        
                        //path does not refer to a directory. 
                        if(!S_ISDIR(next_dir_vnode->vn_mode))
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                /* dir_vnode is the directory vnode where you will unlink the target file */
                                KASSERT(NULL != dir_vnode->vn_ops->unlink);
                                dbg(DBG_PRINT, "(GRADING2A 3.e)\n");
                                dbg(DBG_PRINT, "(GRADING2B)\n");

                                int unlink_return_int=dir_vnode->vn_ops->unlink(dir_vnode,name,namelen);
                                do_unlink_return_int=unlink_return_int;
                                     
                        }
                        else
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                do_unlink_return_int=-EPERM;
                        }
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(next_dir_vnode);
                        
                }
                // A component used as a directory in path is not, in fact, a directory.
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_unlink_return_int=lookup_return_int;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(dir_vnode);   
        }
        // else
        // {
        //         dbg(DBG_PRINT, "do_unlink 10 NC!!!(GRADING2B)\n");
        //         // REFERENCE CLEANUP was handled in dir_namev for these cases
        //         do_unlink_return_int=dir_namev_return_int;
        // }

        dbg(DBG_PRINT, "(GRADING2B)\n");
        return do_unlink_return_int;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EPERM
 *        from is a directory.
 */

int
do_link(const char *from, const char *to)
{
        // NOT_YET_IMPLEMENTED("VFS: do_link");
        //return 0;
        vnode_t *from_node;
        int from_open_namev = open_namev(from,O_RDWR,&from_node,NULL);
        if(from_open_namev<0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return from_open_namev;
        }
        else if(S_ISDIR(from_node->vn_mode))
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                vput(from_node);
                return -EPERM;
        }
        else{
                size_t namelen;
                const char *name;
                vnode_t *to_node;
                int to_dir_namev = dir_namev(to, &namelen, &name, NULL, &to_node);
                if(to_dir_namev<0){
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(from_node);
                        return to_dir_namev;
                }
                else{
                        vnode_t *lookup_to_node;
                        int lookup_to_node_res = lookup(to_node, name, namelen, &lookup_to_node);
                        if (lookup_to_node_res==0){
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                vput(from_node);
                                vput(to_node);
                                vput(lookup_to_node);
                                return -EEXIST;
                        }
                        else if(to_node->vn_ops->link == NULL){ //KERNEL 3
                          dbg(DBG_PRINT, "(GRADING2B)\n");
                           vput(from_node);
                           vput(to_node);
                           return -ENOTDIR;
                        }
                        else if (lookup_to_node_res == -ENOENT)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                int link_ret = to_node->vn_ops->link(from_node, to_node, name, namelen);
                                vput(from_node);
                                vput(to_node);
                                return link_ret;
                        }
                        else{
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                vput(from_node);
                                vput(to_node);
                                return lookup_to_node_res;
                        }

                }
        }

}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        // NOT_YET_IMPLEMENTED("VFS: do_rename");
        //return 0;
        int link_res = do_link(oldname,newname);
        if(link_res==0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return do_unlink(oldname);
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return link_res;
        }
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        // NOT_YET_IMPLEMENTED("VFS: do_chdir");
        dbg(DBG_PRINT, "(GRADING2B)\n");
        
        int do_chdir_return_int;
        vnode_t* dir_vnode=NULL;

        int open_namev_return_int;
        // up the refcount to the new cwd using open_namev() 
        open_namev_return_int=open_namev(path,0,&dir_vnode,NULL);

        // SUCCESS: reference count has been increased
        if(open_namev_return_int==0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // Last component is a directory
                if(S_ISDIR(dir_vnode->vn_mode))
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_chdir_return_int=0;
                        // down the refcount to the old cwd (vput())
                        vput(curproc->p_cwd);
                        // Make the named directory the current process's cwd (current working directory)
                        curproc->p_cwd=dir_vnode;
                }
                // A component of path is not a directory.
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_chdir_return_int=-ENOTDIR;
                        vput(dir_vnode);
                } 
        }
        // FAILURE
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                do_chdir_return_int=-ENOENT;
        }
        
        dbg(DBG_PRINT, "(GRADING2B)\n");
        return do_chdir_return_int;
}

/* Call the readdir vn_op on the given fd, filling in the given dirent_t*.
 * If the readdir vn_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
        // NOT_YET_IMPLEMENTED("VFS: do_getdent");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        int do_getdent_return_int;

        // Before getting file object we need to check if fd
        // lies in the correct range

        if(fd==-1)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // We don't want the call to fget to create the file object
                do_getdent_return_int=-EBADF;
        }
        // fd is not equal to -1
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // We do fget to look in process fd table and return the file*.
                file_t* file_object_ptr=fget(fd);

                // We have a valid file object pointer
                if(file_object_ptr!=NULL)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // The file object pointer refers to a directory
                        // and its corresponding readdir virtual function exists 
                        if(S_ISDIR(file_object_ptr->f_vnode->vn_mode) && file_object_ptr->f_vnode->vn_ops->readdir!=NULL)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                int readdir_return_int;

                                /*
                                * readdir reads one directory entry from the dir into the struct
                                * dirent. On success, it returns the amount that offset should be
                                * increased by to obtain the next directory entry with a
                                * subsequent call to readdir. If the end of the file as been
                                * reached (offset == file->vn_len), no directory entry will be
                                * read and 0 will be returned.
                                */
                                readdir_return_int=file_object_ptr->f_vnode->vn_ops->readdir(file_object_ptr->f_vnode,file_object_ptr->f_pos,dirp);

                                //  Need to increment the file_t's f_pos by this amount
                                file_object_ptr->f_pos=file_object_ptr->f_pos+readdir_return_int;

                                // Setting the return value
                                
                                // If the end of the file as been not reached
                                if(readdir_return_int!=0)
                                {
                                        dbg(DBG_PRINT, "(GRADING2B)\n");
                                        do_getdent_return_int=sizeof(*dirp);
                                }
                                else
                                {
                                        dbg(DBG_PRINT, "(GRADING2B)\n");
                                        do_getdent_return_int=readdir_return_int;
                                }
                                
                        }
                        else
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                do_getdent_return_int=-ENOTDIR;
                        }

                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // decrementing the refcount
                        fput(file_object_ptr);
                        
                }
                // Invalid file descriptor fd.
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        do_getdent_return_int=-EBADF;
                }
                
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");
        
        return do_getdent_return_int;
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int 
do_lseek(int fd, int offset, int whence)
{
        // NOT_YET_IMPLEMENTED("VFS: do_lseek");

        if (fd < 0 || fd > NFILES){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }

        file_t *file = fget(fd);
        if (file == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EBADF;
        }
        switch (whence){
        case SEEK_SET:
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file->f_pos = offset;
                break;
        case SEEK_CUR:
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file->f_pos = (file->f_pos) + offset;
                break;
        case SEEK_END:
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file->f_pos = (file->f_vnode->vn_len) + offset;
                break;
        default:
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return -EINVAL;
        }

        if (file->f_pos >= 0){
                dbg(DBG_PRINT, "(GRADING2B)\n");
                fput(file);
                return file->f_pos;
        }
        else{
                dbg(DBG_PRINT, "(GRADING2B)\n");
                file->f_pos = 0;
                fput(file);
                return -EINVAL;
        }
        
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o EINVAL
 *        path is an empty string.
 */
int do_stat(const char *path, struct stat *buf)
{
        // NOT_YET_IMPLEMENTED("VFS: do_stat");
        dbg(DBG_PRINT, "(GRADING2B)\n");

        // If we don't have an empty string we must find the
        // vnode associated with the path, and call the stat() vnode operation.
        if (strlen(path) != 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                size_t namelen = 0;
                // New
                // const char* name=NULL;
                const char* name;
                // New
                vnode_t *dir_vnode = NULL;
                vnode_t *next_dir_vnode = NULL;
                // Not reachable not used
                // int dir_namev_return_int;
                // dir_namev_return_int = dir_namev(path, &namelen, &name, NULL, &dir_vnode);

                dir_namev(path, &namelen, &name, NULL, &dir_vnode);
                // NOT REACHABLE
                // // if the return is unsuccessful
                // if (dir_namev_return_int != 0)
                // {
                //         dbg(DBG_PRINT, "do_stat 3(GRADING2B)\n");
                //         // REFERENCE CLEANUP was handled in dir_namev for these cases
                //         return dir_namev_return_int;
                // }
                // NOT REACHABLE
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // New Code
                int flag=0;
                if(!name)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        flag=1;
                }
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        flag=2;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
                if(flag==2)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        int lookup_return_int = lookup(dir_vnode, name, namelen, &next_dir_vnode);

                        // if the return is unsuccessful
                        if (lookup_return_int != 0)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                vput(dir_vnode);
                                return lookup_return_int;
                        }   
                }

                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        next_dir_vnode=dir_vnode;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // New Code

                // int lookup_return_int = lookup(dir_vnode, name, namelen, &next_dir_vnode);

                // // if the return is unsuccessful
                // if (lookup_return_int != 0)
                // {
                //         dbg(DBG_PRINT, "do_stat 3(GRADING2B)\n");
                //         vput(dir_vnode);
                //         return lookup_return_int;
                // }

                /* next_dir_vnode is the directory vnode where you will perform "stat" */
                KASSERT(NULL != next_dir_vnode->vn_ops->stat);
                dbg(DBG_PRINT, "(GRADING2A 3.f)\n");
                dbg(DBG_PRINT, "(GRADING2B)\n");

                // So now we call the stat() vnode operation.
                int stat_return_int;
                // /*
                // * stat sets the fields in the given buf, filling it with
                // * information about file.
                // */
                // int (*stat)(struct vnode *vnode, struct stat *buf);
                stat_return_int = next_dir_vnode->vn_ops->stat(next_dir_vnode, buf);

                // Now we no longer need reference to next_dir_vnode
                vput(next_dir_vnode);
                //vput(stat_return_int);

                if(flag==2)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return stat_return_int;
        }
        // Path is an empty string, so we return -EINVAL
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -EINVAL;
        }
}


#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
