#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define HYPARM_PROFILER                 0x48000000
#define HYPARM_PROFILER_READ(reg)       omap_readl(HYPARM_PROFILER + reg)
#define HYPARM_PROFILER_WRITE(reg, val) omap_writel((val), (HYPARM_PROFILER + reg))

#define CONTROL_REG         0x0
#define CONTROL_REG_RUNNING 0x1
#define CONTROL_REG_RESET   0x2
#define SAMPLE_NUM_REG      0x4
#define DATA_REG            0x8

MODULE_AUTHOR("Cosmin Gorgovan");
MODULE_DESCRIPTION("");
MODULE_LICENSE("Dual BSD/GPL");

unsigned int noOfSamples;
unsigned int pointer;

int hyparm_profiler_open(struct inode *inode,struct file *filep)
{
  HYPARM_PROFILER_WRITE(CONTROL_REG, HYPARM_PROFILER_READ(CONTROL_REG) & ~CONTROL_REG_RUNNING);
  noOfSamples = HYPARM_PROFILER_READ(SAMPLE_NUM_REG);
  printk("There are %d PC samples...\n", noOfSamples);
  pointer = 0;

	return 0;
}

int hyparm_profiler_release(struct inode *inode,struct file *filep)
{
  HYPARM_PROFILER_WRITE(CONTROL_REG, HYPARM_PROFILER_READ(CONTROL_REG) | CONTROL_REG_RUNNING);

	return 0;
}
ssize_t hyparm_profiler_read(struct file *filep,char *buff,size_t count,loff_t *offp )
{
  ssize_t size = sizeof(unsigned int);

  if (pointer < noOfSamples) {
    unsigned int value = HYPARM_PROFILER_READ(DATA_REG);

    if ( copy_to_user(buff, (char *)&value, size) != 0 )
	  	printk( "Kernel -> userspace copy failed!\n" );
	  pointer++;
	  return size;
  }

  return 0;
}
ssize_t hyparm_profiler_write(struct file *filep,const char *buff,size_t count,loff_t *offp )
{
  int i;
  for (i = 0; i < count; i++)
  {
    if (buff[i] == 'r')
    {
      HYPARM_PROFILER_WRITE(CONTROL_REG, HYPARM_PROFILER_READ(CONTROL_REG) | CONTROL_REG_RESET);
    }
  }

	return count;
}

struct file_operations hyparm_profiler_fops={
	open:    hyparm_profiler_open,
	read:    hyparm_profiler_read,
	write:   hyparm_profiler_write,
	release: hyparm_profiler_release,
};

static int r_init(void)
{
  if(register_chrdev(222,"hyparm_profiler",&hyparm_profiler_fops)){
	  printk("<3>failed to register");
  }
  printk("<6>hyparm_profiler registered\n");
  
  return 0;
}

static void r_cleanup(void)
{
  unregister_chrdev(222,"hyparm_profiler");
  return;
}

module_init(r_init);
module_exit(r_cleanup);
