#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>

#define IO_BLOCK_SIZE_BIT 12
#define IO_BLOCK_SIZE 4096

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lakshmanan");
MODULE_DESCRIPTION("A Simple module to read block device");

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

void print_a_page(void* addr) {
	char* c = addr;
	// wordline: how many characters are shown in one line
	int wordline = 128;
	char* p = kmalloc(wordline*sizeof(char)+1,GFP_KERNEL);
	int i = 0;
	int j = 0;
	char space = ' ';
	p[wordline]='\0';
	if (c[i]) printk(KERN_INFO "[Block data] (Start with: %c)\n",c[i]);
	else printk(KERN_INFO "[Block data]\n");
	printk(KERN_INFO "----------------\n");
	while (i<IO_BLOCK_SIZE) {
		p[0]='\0';
		for (j=0;j<wordline;j+=32) {
			strncat(p,c+i+j,32);
			strcat(p,&space);
		}
		printk(KERN_INFO "%p %s\n",addr+i,p);
		i+=wordline;
	}
	printk(KERN_INFO "----------------\n");
}

static int __init hello_init(void)
{
	struct file *file;
	loff_t pos = 0;
    struct inode *blk_inode;
	// char* get = kzalloc(sizeof(char)*10, GFP_KERNEL);
    char* c = kzalloc(sizeof(char)*4096, GFP_KERNEL);
    char *bdev_path = find_a_raw_bdev();
    mm_segment_t oldfs;
     
    oldfs = get_fs();
	set_fs(get_ds());
    file = filp_open(bdev_path, O_RDONLY, 0644);
    
	blk_inode = file->f_inode;
    printk(KERN_INFO "vfs read in.\n");
    
    vfs_read(file, c,sizeof(char)*4096, &pos);
    print_a_page(c);

	printk(KERN_INFO "vfs read out.\n");

    filp_close(file,0);

    set_fs(oldfs);
    return 0; 
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);