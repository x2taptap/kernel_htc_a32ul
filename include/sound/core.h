#ifndef __SOUND_CORE_H
#define __SOUND_CORE_H

/*
 *  Main header file for the ALSA driver
 *  Copyright (c) 1994-2001 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/sched.h>		
#include <linux/mutex.h>		
#include <linux/rwsem.h>		
#include <linux/pm.h>			
#include <linux/stringify.h>

#ifdef CONFIG_SND_DYNAMIC_MINORS
#define SNDRV_CARDS 32
#else
#define SNDRV_CARDS 8		
#endif

#define CONFIG_SND_MAJOR	116	

struct pci_dev;
struct module;
struct device;
struct device_attribute;


#define SNDRV_DEV_TYPE_RANGE_SIZE		0x1000

typedef int __bitwise snd_device_type_t;
#define	SNDRV_DEV_TOPLEVEL	((__force snd_device_type_t) 0)
#define	SNDRV_DEV_CONTROL	((__force snd_device_type_t) 1)
#define	SNDRV_DEV_LOWLEVEL_PRE	((__force snd_device_type_t) 2)
#define	SNDRV_DEV_LOWLEVEL_NORMAL ((__force snd_device_type_t) 0x1000)
#define	SNDRV_DEV_PCM		((__force snd_device_type_t) 0x1001)
#define	SNDRV_DEV_RAWMIDI	((__force snd_device_type_t) 0x1002)
#define	SNDRV_DEV_TIMER		((__force snd_device_type_t) 0x1003)
#define	SNDRV_DEV_SEQUENCER	((__force snd_device_type_t) 0x1004)
#define	SNDRV_DEV_HWDEP		((__force snd_device_type_t) 0x1005)
#define	SNDRV_DEV_INFO		((__force snd_device_type_t) 0x1006)
#define	SNDRV_DEV_BUS		((__force snd_device_type_t) 0x1007)
#define	SNDRV_DEV_CODEC		((__force snd_device_type_t) 0x1008)
#define	SNDRV_DEV_JACK          ((__force snd_device_type_t) 0x1009)
#define	SNDRV_DEV_COMPRESS	((__force snd_device_type_t) 0x100A)
#define	SNDRV_DEV_LOWLEVEL	((__force snd_device_type_t) 0x2000)

typedef int __bitwise snd_device_state_t;
#define	SNDRV_DEV_BUILD		((__force snd_device_state_t) 0)
#define	SNDRV_DEV_REGISTERED	((__force snd_device_state_t) 1)
#define	SNDRV_DEV_DISCONNECTED	((__force snd_device_state_t) 2)

typedef int __bitwise snd_device_cmd_t;
#define	SNDRV_DEV_CMD_PRE	((__force snd_device_cmd_t) 0)
#define	SNDRV_DEV_CMD_NORMAL	((__force snd_device_cmd_t) 1)	
#define	SNDRV_DEV_CMD_POST	((__force snd_device_cmd_t) 2)

struct snd_device;

struct snd_device_ops {
	int (*dev_free)(struct snd_device *dev);
	int (*dev_register)(struct snd_device *dev);
	int (*dev_disconnect)(struct snd_device *dev);
};

struct snd_device {
	struct list_head list;		
	struct snd_card *card;		
	snd_device_state_t state;	
	snd_device_type_t type;		
	void *device_data;		
	struct snd_device_ops *ops;	
};

#define snd_device(n) list_entry(n, struct snd_device, list)


struct snd_card {
	int number;			

	char id[16];			
	char driver[16];		
	char shortname[32];		
	char longname[80];		
	char mixername[80];		
	char components[128];		
	struct module *module;		

	void *private_data;		
	void (*private_free) (struct snd_card *card); 
	struct list_head devices;	

	unsigned int last_numid;	
	struct rw_semaphore controls_rwsem;	
	rwlock_t ctl_files_rwlock;	
	int controls_count;		
	int user_ctl_count;		
	struct list_head controls;	
	struct list_head ctl_files;	
	struct mutex user_ctl_lock;	

	struct snd_info_entry *proc_root;	
	struct snd_info_entry *proc_id;	
	struct proc_dir_entry *proc_root_link;	

	struct list_head files_list;	
	struct snd_shutdown_f_ops *s_f_ops; 
	spinlock_t files_lock;		
	int shutdown;			
	int free_on_last_close;		
	wait_queue_head_t shutdown_sleep;
	atomic_t refcount;		
	struct device *dev;		
	struct device *card_dev;	
	int offline;			
	unsigned long offline_change;
	wait_queue_head_t offline_poll_wait;

#ifdef CONFIG_PM
	unsigned int power_state;	
	struct mutex power_lock;	
	wait_queue_head_t power_sleep;
#endif

#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
	struct snd_mixer_oss *mixer_oss;
	int mixer_oss_change_count;
#endif
};

#ifdef CONFIG_PM
static inline void snd_power_lock(struct snd_card *card)
{
	mutex_lock(&card->power_lock);
}

static inline void snd_power_unlock(struct snd_card *card)
{
	mutex_unlock(&card->power_lock);
}

static inline unsigned int snd_power_get_state(struct snd_card *card)
{
	return card->power_state;
}

static inline void snd_power_change_state(struct snd_card *card, unsigned int state)
{
	card->power_state = state;
	wake_up(&card->power_sleep);
}

int snd_power_wait(struct snd_card *card, unsigned int power_state);

#else 

#define snd_power_lock(card)		do { (void)(card); } while (0)
#define snd_power_unlock(card)		do { (void)(card); } while (0)
static inline int snd_power_wait(struct snd_card *card, unsigned int state) { return 0; }
#define snd_power_get_state(card)	({ (void)(card); SNDRV_CTL_POWER_D0; })
#define snd_power_change_state(card, state)	do { (void)(card); } while (0)

#endif 

struct snd_minor {
	int type;			
	int card;			
	int device;			
	const struct file_operations *f_ops;	
	void *private_data;		
	struct device *dev;		
	struct snd_card *card_ptr;	
};

static inline struct device *snd_card_get_device_link(struct snd_card *card)
{
	return card ? card->card_dev : NULL;
}


extern int snd_major;
extern int snd_ecards_limit;
extern struct class *sound_class;

void snd_request_card(int card);

int snd_register_device_for_dev(int type, struct snd_card *card,
				int dev,
				const struct file_operations *f_ops,
				void *private_data,
				const char *name,
				struct device *device);

static inline int snd_register_device(int type, struct snd_card *card, int dev,
				      const struct file_operations *f_ops,
				      void *private_data,
				      const char *name)
{
	return snd_register_device_for_dev(type, card, dev, f_ops,
					   private_data, name,
					   snd_card_get_device_link(card));
}

int snd_unregister_device(int type, struct snd_card *card, int dev);
void *snd_lookup_minor_data(unsigned int minor, int type);
int snd_add_device_sysfs_file(int type, struct snd_card *card, int dev,
			      struct device_attribute *attr);

#ifdef CONFIG_SND_OSSEMUL
int snd_register_oss_device(int type, struct snd_card *card, int dev,
			    const struct file_operations *f_ops, void *private_data,
			    const char *name);
int snd_unregister_oss_device(int type, struct snd_card *card, int dev);
void *snd_lookup_oss_minor_data(unsigned int minor, int type);
#endif

int snd_minor_info_init(void);
int snd_minor_info_done(void);


#ifdef CONFIG_SND_OSSEMUL
int snd_minor_info_oss_init(void);
int snd_minor_info_oss_done(void);
#else
static inline int snd_minor_info_oss_init(void) { return 0; }
static inline int snd_minor_info_oss_done(void) { return 0; }
#endif


int copy_to_user_fromio(void __user *dst, const volatile void __iomem *src, size_t count);
int copy_from_user_toio(volatile void __iomem *dst, const void __user *src, size_t count);


extern struct snd_card *snd_cards[SNDRV_CARDS];
int snd_card_locked(int card);
#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
#define SND_MIXER_OSS_NOTIFY_REGISTER	0
#define SND_MIXER_OSS_NOTIFY_DISCONNECT	1
#define SND_MIXER_OSS_NOTIFY_FREE	2
extern int (*snd_mixer_oss_notify_callback)(struct snd_card *card, int cmd);
#endif

int snd_card_create(int idx, const char *id,
		    struct module *module, int extra_size,
		    struct snd_card **card_ret);

int snd_card_disconnect(struct snd_card *card);
int snd_card_free(struct snd_card *card);
int snd_card_free_when_closed(struct snd_card *card);
void snd_card_set_id(struct snd_card *card, const char *id);
int snd_card_register(struct snd_card *card);
int snd_card_info_init(void);
int snd_card_info_done(void);
int snd_component_add(struct snd_card *card, const char *component);
int snd_card_file_add(struct snd_card *card, struct file *file);
int snd_card_file_remove(struct snd_card *card, struct file *file);
void snd_card_unref(struct snd_card *card);
void snd_card_change_online_state(struct snd_card *card, int online);
bool snd_card_is_online_state(struct snd_card *card);

#define snd_card_set_dev(card, devptr) ((card)->dev = (devptr))


int snd_device_new(struct snd_card *card, snd_device_type_t type,
		   void *device_data, struct snd_device_ops *ops);
int snd_device_register(struct snd_card *card, void *device_data);
int snd_device_register_all(struct snd_card *card);
int snd_device_disconnect(struct snd_card *card, void *device_data);
int snd_device_disconnect_all(struct snd_card *card);
int snd_device_free(struct snd_card *card, void *device_data);
int snd_device_free_all(struct snd_card *card, snd_device_cmd_t cmd);


#ifdef CONFIG_ISA_DMA_API
#define DMA_MODE_NO_ENABLE	0x0100

void snd_dma_program(unsigned long dma, unsigned long addr, unsigned int size, unsigned short mode);
void snd_dma_disable(unsigned long dma);
unsigned int snd_dma_pointer(unsigned long dma, unsigned int size);
#endif

struct resource;
void release_and_free_resource(struct resource *res);


enum {
	SND_PR_ALWAYS,
	SND_PR_DEBUG,
	SND_PR_VERBOSE,
};

#if defined(CONFIG_SND_DEBUG) || defined(CONFIG_SND_VERBOSE_PRINTK)
__printf(4, 5)
void __snd_printk(unsigned int level, const char *file, int line,
		  const char *format, ...);
#else
#define __snd_printk(level, file, line, format, args...) \
	printk(format, ##args)
#endif

#define snd_printk(fmt, args...) \
	__snd_printk(0, __FILE__, __LINE__, fmt, ##args)

#ifdef CONFIG_SND_DEBUG
#define snd_printd(fmt, args...) \
	__snd_printk(1, __FILE__, __LINE__, fmt, ##args)
#define _snd_printd(level, fmt, args...) \
	__snd_printk(level, __FILE__, __LINE__, fmt, ##args)

#define snd_BUG()		WARN(1, "BUG?\n")

#define snd_BUG_ON(cond)	WARN_ON((cond))

#else 

__printf(1, 2)
static inline void snd_printd(const char *format, ...) {}
__printf(2, 3)
static inline void _snd_printd(int level, const char *format, ...) {}

#define snd_BUG()			do { } while (0)

#define snd_BUG_ON(condition) ({ \
	int __ret_warn_on = !!(condition); \
	unlikely(__ret_warn_on); \
})

#endif 

#ifdef CONFIG_SND_DEBUG_VERBOSE
#define snd_printdd(format, args...) \
	__snd_printk(2, __FILE__, __LINE__, format, ##args)
#else
__printf(1, 2)
static inline void snd_printdd(const char *format, ...) {}
#endif


#define SNDRV_OSS_VERSION         ((3<<16)|(8<<8)|(1<<4)|(0))	

#if defined(CONFIG_GAMEPORT) || defined(CONFIG_GAMEPORT_MODULE)
#define gameport_set_dev_parent(gp,xdev) ((gp)->dev.parent = (xdev))
#define gameport_set_port_data(gp,r) ((gp)->port_data = (r))
#define gameport_get_port_data(gp) (gp)->port_data
#endif

#ifdef CONFIG_PCI
struct snd_pci_quirk {
	unsigned short subvendor;	
	unsigned short subdevice;	
	unsigned short subdevice_mask;	
	int value;			
#ifdef CONFIG_SND_DEBUG_VERBOSE
	const char *name;		
#endif
};

#define _SND_PCI_QUIRK_ID_MASK(vend, mask, dev)	\
	.subvendor = (vend), .subdevice = (dev), .subdevice_mask = (mask)
#define _SND_PCI_QUIRK_ID(vend, dev) \
	_SND_PCI_QUIRK_ID_MASK(vend, 0xffff, dev)
#define SND_PCI_QUIRK_ID(vend,dev) {_SND_PCI_QUIRK_ID(vend, dev)}
#ifdef CONFIG_SND_DEBUG_VERBOSE
#define SND_PCI_QUIRK(vend,dev,xname,val) \
	{_SND_PCI_QUIRK_ID(vend, dev), .value = (val), .name = (xname)}
#define SND_PCI_QUIRK_VENDOR(vend, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, 0, 0), .value = (val), .name = (xname)}
#define SND_PCI_QUIRK_MASK(vend, mask, dev, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, mask, dev),			\
			.value = (val), .name = (xname)}
#define snd_pci_quirk_name(q)	((q)->name)
#else
#define SND_PCI_QUIRK(vend,dev,xname,val) \
	{_SND_PCI_QUIRK_ID(vend, dev), .value = (val)}
#define SND_PCI_QUIRK_MASK(vend, mask, dev, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, mask, dev), .value = (val)}
#define SND_PCI_QUIRK_VENDOR(vend, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, 0, 0), .value = (val)}
#define snd_pci_quirk_name(q)	""
#endif

const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list);

const struct snd_pci_quirk *
snd_pci_quirk_lookup_id(u16 vendor, u16 device,
			const struct snd_pci_quirk *list);
#endif

#endif 
