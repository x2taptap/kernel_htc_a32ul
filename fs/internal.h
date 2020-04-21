/* fs/ internal definitions
 *
 * Copyright (C) 2006 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/lglock.h>

struct super_block;
struct file_system_type;
struct linux_binprm;
struct path;
struct mount;

#ifdef CONFIG_BLOCK
extern void __init bdev_cache_init(void);

extern int __sync_blockdev(struct block_device *bdev, int wait);

#else
static inline void bdev_cache_init(void)
{
}

static inline int __sync_blockdev(struct block_device *bdev, int wait)
{
	return 0;
}
#endif

extern void __init chrdev_init(void);

extern int __inode_permission(struct inode *, int);

extern int copy_mount_options(const void __user *, unsigned long *);
extern int copy_mount_string(const void __user *, char **);

extern struct vfsmount *lookup_mnt(struct path *);
extern int finish_automount(struct vfsmount *, struct path *);

extern int sb_prepare_remount_readonly(struct super_block *);

extern void __init mnt_init(void);

extern struct lglock vfsmount_lock;

extern int __mnt_want_write(struct vfsmount *);
extern int __mnt_want_write_file(struct file *);
extern void __mnt_drop_write(struct vfsmount *);
extern void __mnt_drop_write_file(struct file *);

extern void chroot_fs_refs(const struct path *, const struct path *);

extern void file_sb_list_add(struct file *f, struct super_block *sb);
extern void file_sb_list_del(struct file *f);
extern void mark_files_ro(struct super_block *);
extern struct file *get_empty_filp(void);

extern int do_remount_sb(struct super_block *, int, void *, int);
extern bool grab_super_passive(struct super_block *sb);
extern struct dentry *mount_fs(struct file_system_type *,
			       int, const char *, void *);
extern struct super_block *user_get_super(dev_t);
extern int umount2(char *name, int flags);

struct open_flags {
	int open_flag;
	umode_t mode;
	int acc_mode;
	int intent;
};
extern struct file *do_filp_open(int dfd, struct filename *pathname,
		const struct open_flags *op, int flags);
extern struct file *do_file_open_root(struct dentry *, struct vfsmount *,
		const char *, const struct open_flags *, int lookup_flags);

extern long do_handle_open(int mountdirfd,
			   struct file_handle __user *ufh, int open_flag);
extern int open_check_o_direct(struct file *f);

extern spinlock_t inode_sb_list_lock;
extern void inode_add_lru(struct inode *inode);

extern void inode_wb_list_del(struct inode *inode);

extern int get_nr_dirty_inodes(void);
extern void evict_inodes(struct super_block *);
extern int invalidate_inodes(struct super_block *, bool);

extern struct dentry *__d_alloc(struct super_block *, const struct qstr *);

extern ssize_t __kernel_write(struct file *, const char *, size_t, loff_t *);

extern long do_splice_direct(struct file *in, loff_t *ppos, struct file *out,
		loff_t *opos, size_t len, unsigned int flags);

extern const struct file_operations pipefifo_fops;
