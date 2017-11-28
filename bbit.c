#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/genhd.h>

#define IO_BLOCK_SIZE_BIT 12
#define IO_BLOCK_SIZE 4096

static long index = 0;

static bool all = false;

struct block_device *bdev_raw;	
struct gendisk*	bd_disk = NULL;
unsigned long nsector = 0;
unsigned long capacity_page = 0;
char* c = NULL;
loff_t offset = 0;
struct file *bdev;
struct file *file;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("venero");
MODULE_DESCRIPTION("A Simple module to read block device");

module_param(index, long, S_IRUSR);
MODULE_PARM_DESC(index, "Index");

module_param(all, bool, S_IRUSR);
MODULE_PARM_DESC(all, "Whether print all");

char* find_a_raw_bdev(void) {
	struct file *fp;
	char* bdev = kzalloc(20*sizeof(char),GFP_KERNEL);
		
	fp = filp_open("/dev/sda1", O_RDONLY, 0644);
	if(fp == (struct file *)-ENOENT) {
		strcat(bdev, "/dev/sda\0");
		printk(KERN_INFO "[sda] is chosen.\n");
		return bdev;
	}
	fp = filp_open("/dev/sdb1", O_RDONLY, 0644);
	if(fp == (struct file *)-ENOENT) {
		strcat(bdev, "/dev/sdb\0");
		printk(KERN_INFO "[sdb] is chosen.\n");
		return bdev;
	}
	return NULL;
}

void print_a_block(long idx) {
	int wordline = 128;
	char* p = kmalloc(wordline*sizeof(char)+1,GFP_KERNEL);	
	int i = 0;
	int j = 0;
	char space = ' ';
	int ret = 0;
	char *buffer = kmalloc(5000,GFP_KERNEL);
		
	loff_t pos = (idx << IO_BLOCK_SIZE_BIT);
	p[wordline]='\0';
	vfs_read(file, c,sizeof(char)*4096, &pos);
	
	while (i<IO_BLOCK_SIZE) {
		p[0]='\0';
		for (j=0;j<wordline;j+=32) {
			strncat(p,c+i+j,32);
			strcat(p,&space);
		}
		sprintf(buffer, "%s\n",p);
		ret = vfs_write(file, buffer, strlen(buffer), &offset);

		i+=wordline;
	}
}

void print_block(void) {
	// wordline: how many characters are shown in one line
	int k = 0;
	mm_segment_t oldfs;
	int ret = 0;

	char *file_path = kmalloc(100,GFP_KERNEL);
	char *buffer = kmalloc(100,GFP_KERNEL);
	
    oldfs = get_fs();
    set_fs(get_ds());
	

	if (all) {
		sprintf(file_path, "./block_all");
    	file = filp_open(file_path, O_CREAT|O_WRONLY , 0644);
		printk(KERN_INFO "File: %s\n", file_path);
		while (k<capacity_page)	print_a_block(k++);
	}
	else {
		sprintf(file_path, "./block_#%ld", index);
    	file = filp_open(file_path, O_CREAT|O_WRONLY , 0644);
		printk(KERN_INFO "File: %s\n", file_path);

		if (c[0]) {
			sprintf(buffer, "[Block data] (Start with: %c)\n",c[0]);
			ret = vfs_write(file, buffer, strlen(buffer), &offset);
		}
		else {
			sprintf(buffer, "[Block data]\n");
			ret = vfs_write(file, buffer, strlen(buffer), &offset);
		}

		sprintf(buffer, "\n----------------\n\n");
		ret = vfs_write(file, buffer, strlen(buffer), &offset);

		print_a_block(index);

		sprintf(buffer, "\n----------------\n");
		ret = vfs_write(file, buffer, strlen(buffer), &offset);

	}
	set_fs(oldfs);
	filp_close(file, 0);
	kfree(file_path);
	kfree(buffer);
}

static int __init hello_init(void)
{
	loff_t pos = index << IO_BLOCK_SIZE_BIT;
	struct inode *blk_inode;
	const fmode_t mode = FMODE_READ | FMODE_WRITE;

	// char* get = kzalloc(sizeof(char)*10, GFP_KERNEL);

    char *bdev_path = find_a_raw_bdev();
    mm_segment_t oldfs;
		
	c = kzalloc(sizeof(char)*4096, GFP_KERNEL);
	
	bdev_raw = lookup_bdev(bdev_path);
	if (IS_ERR(bdev_raw))
	{
		printk(KERN_INFO "bdev: error opening raw device <%lu>\n", PTR_ERR(bdev_raw));
	}
	if (!bdget(bdev_raw->bd_dev))
	{
		printk(KERN_INFO "bdev: error bdget()\n");
	}
	if (blkdev_get(bdev_raw, mode, NULL))
	{
		printk(KERN_INFO "bdev: error blkdev_get()\n");
		bdput(bdev_raw);
	}	

	bd_disk = bdev_raw->bd_disk;
	nsector = get_capacity(bd_disk);	
	capacity_page = nsector>>3;

	printk(KERN_INFO "index: %lu\n", index);

    oldfs = get_fs();
	set_fs(get_ds());
    bdev = filp_open(bdev_path, O_RDONLY, 0644);
    
	blk_inode = bdev->f_inode;
    printk(KERN_INFO "vfs read in.\n");
    
    vfs_read(bdev, c,sizeof(char)*4096, &pos);
    print_block();

	printk(KERN_INFO "vfs read out.\n");

    filp_close(bdev,0);

	set_fs(oldfs);
	
	kfree(c);
    return 0; 
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);