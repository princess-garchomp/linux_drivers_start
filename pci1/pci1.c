#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>

#include <linux/fs.h>
#include <asm/uaccess.h>        /* for put_user */


#define DEVICE_NAME "chardev"  
#define SUCCESS 0
#define BUF_LEN 80              /* Max length of the message from the device */



//PP means parallel port
#define PP_CLASS		(0x070103)
//#define PP_CLASS_MASK
#define PP_VENDOR		(0x00009710)
#define PP_DEVICE		(0x00009900)
#define PP_SUB_VENDOR		(0x0000A000)
#define PP_SUB_DEVICE		(0x00002000)

//to get this address I used the folloing command 
//lspci -vvnn -s 07:00
//
//at the moment I am manually fiding this address
#define PP_ADDRESS_START	(0xd010)

static char msg[BUF_LEN];       /* The msg the device will give when asked */
static char *msg_Ptr;

static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open? */ 

unsigned long address_start;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct pci_device_id ids[] = {
	{ .class = PP_CLASS, .vendor = PP_VENDOR, .device = PP_DEVICE, .subvendor = PP_SUB_VENDOR, .subdevice = PP_SUB_DEVICE },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

static unsigned char skel_get_revision(struct pci_dev *dev)
{
	//emty for now
	//
	u8 revision;
	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	return revision;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	//empty for now
	//
	if(pci_enable_device(dev)) {
		dev_err(&dev->dev, "can't enable PCI device\n");
		return -ENODEV;
	}
	if (skel_get_revision(dev) == 0x42)
		return -ENODEV;

	return 0;
}

static void remove(struct pci_dev *dev)
{
	// empty for now
	//
}

static struct pci_driver pci_driver = {
	.name = "pci_card_parallel_port",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};


static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
	.open = device_open,
        .release = device_release

};

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
        static int counter = 0;
        static int counter_2 = 0;

        if (Device_Open)
                return -EBUSY;

        Device_Open++;
        sprintf(msg, "I already told you %d times Hello world!\n", counter++);

	if(counter_2==1)
	{
        	outb(0xFF, PP_ADDRESS_START);
        	wmb( );
	}
	else
	{
       		outb(0x00,PP_ADDRESS_START);
	        wmb( );
	}

        msg_Ptr = msg;
        try_module_get(THIS_MODULE);
	
	counter_2=(counter_2+1)%3;
        return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
        Device_Open--;          /* We're now ready for our next caller */

        /*
         * Decrement the usage count, or else once you opened the file, you'll
         * never get get rid of the module.
         */
        module_put(THIS_MODULE);

        return 0;
}

static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
        /*
         * Number of bytes actually written to the buffer
         */
        int bytes_read = 0;

        /*
         * If we're at the end of the message,
         * return 0 signifying end of file
         */
        if (*msg_Ptr == 0)
                return 0;

        /*
         * Actually put the data into the buffer
         */
        while (length && *msg_Ptr) {

                /*
                 * The buffer is in the user data segment, not the kernel
                 * segment so "*" assignment won't work.  We have to use
                 * put_user which copies data from the kernel data segment to
                 * the user data segment.
                 */
                put_user(*(msg_Ptr++), buffer++);

                length--;
                bytes_read++;
        }

        /*
         * Most read functions return the number of bytes put into the buffer
         */
        return bytes_read;
}
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
        printk(KERN_ALERT " I dont do that\n");
        return -EINVAL;
}



static int __init pci1_init(void)
{

	int register_return_value;
	//this fucniton returns a 0, if it error occured it returns a negative value

	//sudo cat /proc/ioports | grep pci_card_parallel_port
	//the above line will show if the memory region was requested
	if (!request_region(PP_ADDRESS_START, 2,"pci_card_parallel_port"))	
	{
       		printk(KERN_INFO "coulr not reserve the location for you\n");
        	return -ENODEV;
        }

	//lspci -vvnn -s 07:00 
	//now says:
		//Kernel driver in use: pci_card_parallel_port

	register_return_value =  pci_register_driver(&pci_driver);


        if (register_return_value < 0) 
	{
          printk(KERN_ALERT "Registering the PCI driver failed %d\n pci_register_driver returned a negative number \n", register_return_value);
        }
	else
	{	
        printk(KERN_ALERT "Registering the PCI driver succeeded, value is  %d\n", register_return_value);
	}

//	address_start = pci_resource_start(dev, 1);
//        printk(KERN_ALERT "the first memory address is  %ld\n", address_start);
//
 	//un-comment this to turn the LEDS on when the module is loaded into the kernal
       // outb(0xFF, PP_ADDRESS_START);
       // wmb( );
        Major = register_chrdev(0, DEVICE_NAME, &fops);

        if (Major < 0) {
          printk(KERN_ALERT "Registering char device failed with %d\n", Major);
          return Major;
        }

        printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
        printk(KERN_INFO "the driver, create a dev file with\n");
        printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
        printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
        printk(KERN_INFO "the device file.\n");
        printk(KERN_INFO "Remove the device file and module when done.\n");


	return register_return_value;
}

static void __exit pci1_exit(void)
{
        outb(0x00,PP_ADDRESS_START);
        wmb( );

	release_region(PP_ADDRESS_START, 2);
	pci_unregister_driver(&pci_driver);
        unregister_chrdev(Major, DEVICE_NAME);
        printk(KERN_ALERT "PIC driver and chardev un-registerd\n");
}

MODULE_LICENSE("GPL");

module_init(pci1_init);
module_exit(pci1_exit);


//a usefull file location is /sys/bus/pci/devices/
//the modalias file has the sub-device value
