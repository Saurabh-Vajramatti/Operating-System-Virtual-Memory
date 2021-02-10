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

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
        // NOT_YET_IMPLEMENTED("VFS: lookup");
        KASSERT(NULL != dir); /* the "dir" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");

        KASSERT(NULL != name); /* the "name" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");

        KASSERT(NULL != result); /* the "result" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");

        // K3NC
        if (len == 0)	
         {	
                 /*dbg(DBG_TEST, "--------------------------323\n");*/	
                dbg(DBG_PRINT, "(GRADING2B)\n");	
                *result = dir;	
                vref(*result);	
                return 0;	
         }
        // K3NC


        // /*
        //  * Function pointers to the implementations of file operations (the
        //  * functions are provided by the filesystem implementation).
        //  */
        // struct vnode_ops   *vn_ops;

        // /* All functions under struct vnode_ops Eg. lookup map directly to their corresponding
        //  * system calls. Unless otherwise noted, they return 0 on
        //  * success, and -errno on failure.
        //  */

        // /*
        //  * lookup sets *result to the vnode in dir with the specified name.
        //  */
        // int (*lookup)(struct vnode *dir, const char *name, size_t name_len,
        //               struct vnode **result);

        // If dir has no lookup(), return -ENOTDIR.

       
        dbg(DBG_PRINT, "(GRADING2B)\n");
        if(dir->vn_ops->lookup==NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return -ENOTDIR;
        }
        // New code
        if(name != NULL && strcmp(name, ".") == 0)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                *result = dir;
                vref(dir);
                return 0;
        }
        // New code

        int lookup_return_int;
        /* All functions under struct vnode_ops Eg. lookup map directly to their corresponding
         * system calls. Unless otherwise noted, they return 0 on
         * success, and -errno on failure.
         */
        lookup_return_int=dir->vn_ops->lookup(dir,name,len,result);

        // dbg(DBG_PRINT, "lookup 4(GRADING2B) %d\n",lookup_return_int);

        dbg(DBG_PRINT, "(GRADING2B)\n");

        return lookup_return_int;
}

/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int dir_namev(const char *pathname, size_t *namelen, const char **name,
              vnode_t *base, vnode_t **res_vnode)
{
        // NOT_YET_IMPLEMENTED("VFS: dir_namev");
        KASSERT(NULL != pathname); /* the "pathname" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != namelen); /* the "namelen" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != name); /* the "name" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        KASSERT(NULL != res_vnode); /* the "res_vnode" argument must be non-NULL */
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        vnode_t *dir_vnode;

        // // K3NC
        // if(!strcmp(pathname,"")){	
        //     dbg(DBG_PRINT, "(GRADING2B)\n");	
        //     return -EINVAL;	
        // }
        // // K3NC

        dbg(DBG_PRINT, "(GRADING2B)\n");
        // When If pathname[0] != '/' a base value of NULL means to use the process's current working directory, curproc->p_cwd.
        if (pathname[0] != '/' && base == NULL)
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                dir_vnode = curproc->p_cwd;
                // Incrementing reference count
                vref(dir_vnode);
        }
        // Not reachable
        // When If pathname[0] != '/' a base value of not NULL means to use the base as initial_base
        // else if (pathname[0] != '/' && base != NULL)
        // {
        //         dbg(DBG_PRINT, "dir_namev 3(GRADING2B)\n");
        //         dir_vnode = base;
        //         // Incrementing reference count
        //         vref(dir_vnode);
        // }
        // Not Reachable

        // If pathname[0] == '/', ignore base and start with vfs_root_vn
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                dir_vnode = vfs_root_vn;
                // Incrementing reference count
                vref(dir_vnode);
        }
        dbg(DBG_PRINT, "(GRADING2B)\n");

        KASSERT(NULL != dir_vnode); /* pathname resolution must start with a valid directory */
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        // Finding the end of the path
        int end_of_path_index = 0;

        for (int i = strlen(pathname) - 1;; i--)
        {
                if(i<0)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                       end_of_path_index = i;
                       break;
                }
                if (pathname[i] != '/')
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        end_of_path_index = i;
                        break;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        // #define NAME_LEN 28  /* maximum directory entry length */

        // PATH RESOLUTION
        // dbg(DBG_PRINT, "end_of_path_index = %d\n", end_of_path_index);
        dbg(DBG_PRINT, "(GRADING2B)\n");
        vnode_t *next_dir_vnode = NULL;
        int i = 0;
        for (; i <= end_of_path_index;)
        {
                if (pathname[i] == '/')
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        i += 1;
                        continue;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");

                int file_name_length = 0;
                int file_name_start_index = i;
                while (pathname[i] != '/')
                {
                        // if (i > end_of_path_index)
                        // {
                        //         --i;
                        //         break;
                        // }
                        // dbg(DBG_PRINT, " %d %s %c dir_namev 8(GRADING2B)\n", i, pathname, pathname[i]);
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        file_name_length += 1;
                        i += 1;
                        // New Code
                        if (i > end_of_path_index)
                        {
                                // dbg(DBG_PRINT, " %d %s %c dir_namev N8.1(GRADING2B)\n", i, pathname, pathname[i]);
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                --i;
                                break;
                        }
                        // New code
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                }

                // ENAMETOOLONG: A component of path was too long.
                if (file_name_length > NAME_LEN)
                {
                        // Decrementing reference count
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                        return -ENAMETOOLONG;
                }

                // Reached the last file in the pathname
                if (i == end_of_path_index)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        *name = &pathname[file_name_start_index];
                        *res_vnode = dir_vnode;
                        *namelen = file_name_length;
                        break;
                }

                dbg(DBG_PRINT, "(GRADING2B)\n");
                int lookup_return_int = lookup(dir_vnode, &pathname[file_name_start_index], file_name_length, &next_dir_vnode);
                if (lookup_return_int == 0)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                        dir_vnode = next_dir_vnode;
                        KASSERT(NULL != dir_vnode); /* pathname resolution must start with a valid directory */
                        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
                }
                else
                {
                        // Decrementing reference count
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                        return lookup_return_int;
                }
                dbg(DBG_PRINT, "(GRADING2B)\n");
        }

        // NEW Code K3NC
        if(end_of_path_index<0)
        {
                dbg(DBG_PRINT,"(GRADING2B)\n");
                *name = pathname;
                *res_vnode = dir_vnode;
                *namelen = 0;
        }
        // New Code K3NC

        dbg(DBG_PRINT, "(GRADING2B)\n");
        return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        // NOT_YET_IMPLEMENTED("VFS: open_namev");

        dbg(DBG_PRINT, "(GRADING2B)\n");
        size_t namelen=0;
        const char* name=NULL;
        vnode_t* dir_vnode=NULL;
        int dir_namev_return_int;
        dir_namev_return_int= dir_namev(pathname, &namelen, &name, base, &dir_vnode);

        // if the return is successful
        if(dir_namev_return_int==0)
        {
                // // Not reachable
                // // New Code
                // if(!name)
                // {
                //         *res_vnode = dir_vnode;
                //         dbg(DBG_PRINT,"open_namev 2(GRADING2B)\n");
                //         return 0;
                // }
                // // New code
                // // Not reachable

                dbg(DBG_PRINT, "(GRADING2B)\n");
                // So we do lookup
                int lookup_return_int=lookup(dir_vnode,name,namelen,res_vnode);

                // if the return is successful
                if(lookup_return_int==0)
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // Just release reference for dir_vnode and not next_dir_vnode
                        vput(dir_vnode);
                        return 0;
                }
                // if the return is unsuccessful
                else
                {
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        // /* File status flags for open(). */
                        // #define O_CREAT         0x100   /* Create file if non-existent. */
                        // #define O_TRUNC         0x200   /* Truncate to zero length. */
                        // #define O_APPEND        0x400   /* Append to file. */

                        // If lookup_return_int!=-ENOTDIR it means that the only
                        // remaining possibility is that the file does not exist
                        if(lookup_return_int!=-ENOTDIR)
                        {
                                dbg(DBG_PRINT, "(GRADING2B)\n");
                                // Specifically open_namev checks for O_CREAT flag,
                                // if we don't send O_CREAT it will not create the
                                // file if it does not exist
                                if ((flag & O_CREAT)) 
                                {       
                                        dbg(DBG_PRINT, "(GRADING2B)\n");
                                        /* if file does not exist inside dir_vnode, need to make sure you can create the file */
                                        KASSERT(NULL != dir_vnode->vn_ops->create);
                                        dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                                        dbg(DBG_PRINT, "(GRADING2B)\n");

                                        // So, file does not exist and we have been directed to create it

                                        int create_return_int=dir_vnode->vn_ops->create(dir_vnode,name,namelen,res_vnode);
                                        
                                        vput(dir_vnode);
                                        return create_return_int;
                                }
                        }
                        dbg(DBG_PRINT, "(GRADING2B)\n");
                        vput(dir_vnode);
                        return lookup_return_int;
                }
        }
        // if the return is unsuccessful
        else
        {
                dbg(DBG_PRINT, "(GRADING2B)\n");
                // REFERENCE CLEANUP was handled in dir_namev for these cases
                return dir_namev_return_int;
        }
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
