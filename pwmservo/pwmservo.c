/* PWMServo - PWM Controlled Servo Driver for the PXA270 (Gumstix Verdex)
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: Thu May 2, 2013
 *
 * Base PWM Frequency is 13MHz, but can be divided using the PWM_PWCTRLx and
 * PWM_PERVALx to set the prescaler and period length. With both set to the
 * largest value the PWM period can be is 5ms. This is a faster period 
 * than the expected RC Servo PWM frame period of 20ms. The standard is forgiving
 * of 5ms period, but there is a high power draw and it SEVERLY reduces the servos
 * lifetime. To solve this problem a OS Timer on Match 1 is used to help toggle the
 * PWM signal on and off. The scheme is to let about 4 signals through every 100ms.
 * This scheme has been shown to work in situations with low torque requirements and
 * provides stable motion.
 */

/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/jiffies.h> /* jiffies */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <asm/arch/gpio.h>
#include <asm/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <linux/clocksource.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/mach/time.h>

#define TOGGLE_TICK 80000
 // @TODO figure out resolution maths
#define PWMSERVO_MAJOR 61

/** Define module metadata */
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Christopher J. Woodall <chris.j.woodall@gmail.com>");

/** Define pwmservo file ops */
/* Define mytimer file operator function headers (open, release, read, write) */
static int pwmservo_open(struct inode *inode, struct file *filp);
static int pwmservo_release(struct inode *inode, struct file *filp);
static ssize_t pwmservo_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t pwmservo_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* pwmservo file access structure */
struct file_operations pwmservo_fops = {
    read: pwmservo_read,
    write: pwmservo_write,
    open: pwmservo_open,
    release: pwmservo_release
};

/* Prototypes */
static int pwmservo_init(void);
static void pwmservo_exit(void);

/* Declaration of the init and exit functions */
module_init(pwmservo_init);
module_exit(pwmservo_exit);

// Setup servo value global variables.
uint32_t servo_val0 = 0;
uint32_t servo_val1 = 0;
uint32_t servo_cycles = 0;

/**
 * static irqreturn_t servo_timer_handler(int, void) 
 *
 * Interrupt handler for OSTimer1 Interrupts.
 *
 * @param  irq  IRQ vector id.
 * @param  void *  dev_id  The device ID passed by the IRQ request, in this case NULL
 * @return  irqreturn_t  Returns status of IRQ. Always returns IRQ_HANDLED
 */
static irqreturn_t servo_timer_handler(int irq, void *dev_id) {
    /*    printk(KERN_ALERT "HANDLING!");
          printk(KERN_ALERT "%i", OSCR);*/
    int next_match;
    
    do {
        OSSR = OSSR_M1;  /* Clear match on timer 1 */
        next_match = (OSMR1 += TOGGLE_TICK);
        servo_cycles += 1;
    } while( (signed long)(next_match - OSCR) <= 8 );
    
    // Send servo_valN to the appropriate PWM channel when the time is right.
    PWM_PWDUTY0 = ((servo_cycles & 0x3) == 0x3)?servo_val0:0;
    PWM_PWDUTY1 = ((servo_cycles & 0x3) == 0x3)?servo_val1:0;

    // Clever alternative
    // servo_cycles
    return IRQ_HANDLED;
}

static int pwmservo_init(void) {
    /** Register character driver for pwmservo **/
    int ret;
    ret = register_chrdev(PWMSERVO_MAJOR, "pwmservo", &pwmservo_fops);

    if (ret < 0) {
        printk(KERN_ALERT "Woah! You failed to register the pwmservo char device...\n");
        return ret;
    }
    
    /** Setup GPIOs for PWM0 and PWM1 **/
    pxa_gpio_mode(GPIO16_PWM0_MD); // setup GPIO16 as PWM0
    pxa_set_cken(CKEN0_PWM0,1); //Enable the PWM0 Clock
    servo_val0 = 0;
    PWM_PWDUTY0 = servo_val0;
    PWM_CTRL0 = 0x3F;
    PWM_PERVAL0 = 0x3FF;

    pxa_gpio_mode(GPIO17_PWM1_MD); // setup GPIO17 as PWM1
    pxa_set_cken(CKEN1_PWM1,1); //Enable the PWM1 Clock
    servo_val1 = 0;
    PWM_PWDUTY1 = servo_val1;
    PWM_CTRL1 = 0x3F;
    PWM_PERVAL1 = 0x3FF;
    
    // Initialize OS TIMER
    //    OSSR = 0xf; /* clear status on all timers */
    if (request_irq(IRQ_OST1, &servo_timer_handler, IRQF_TIMER | IRQF_DISABLED, "", NULL)) {
        goto fail;
    }

    OIER |= OIER_E1; /* enable match on timer match 1 to cause interrupts */
    // Install OS Timer Match 1 interrupt
    OSMR1 = OSCR + TOGGLE_TICK; /* set initial match */

    printk(KERN_ALERT "Installed PWM Servo");
    return 0;
fail: 
	pwmservo_exit(); 
	return 0;
}


static void pwmservo_exit(void) {
	/* Free memory */	
    unregister_chrdev(PWMSERVO_MAJOR, "pwmservo");

    // Disable and release PWM0 and PWM1 clocks
    pxa_set_cken(CKEN0_PWM0,0); //Enable the PWM0 Clock
    pxa_set_cken(CKEN1_PWM1,0); //Disable the PWM1 Clock

    OIER = OIER & (~OIER_E1); /* disable match on timer match 1 to cause interrupts */
    free_irq(IRQ_OST1, NULL);
	printk(KERN_ALERT "Removing pwmservo module\n");
}


// File Ops
/** Implement file operators **/
static int pwmservo_open(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}

static int pwmservo_release(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}

// Present status information to userland
static ssize_t pwmservo_read( struct file *filp, char *buf, size_t count, 
                            loff_t *f_pos )
{
    /* success... kind of */
    return 0;
}

static ssize_t pwmservo_write(struct file *filp, const char *buf, size_t count,
                            loff_t *f_pos)
{
    char input_buffer[8];    
    uint32_t res;
     // Copy write buffer into kernel memory
     if (copy_from_user(input_buffer, buf, 8)) { 
         return -EFAULT; 
     }

     // Read input, only cares about tNUM and pNUM formatted inputs for controlling pan and tilt
     if (input_buffer[0] == 't') {
         // Set tilt servo value, only accept numbers between 200 and 410. All other numbers
         // may cause damage to the servo.
         res = simple_strtol((input_buffer+1), NULL, 10);
         if (((res > 199) && (res < 411)) || (res == 0)){ // Between 200 and 410
             servo_val0 = res;
         }
     } else if (input_buffer[0] =='p') {
         // Set pan servo value, only accept numbers between 200 and 410. All other numbers
         // may cause damage to the servo by driving it to a location it is mechanically stopped
         // from and stripping the gears.
         res = simple_strtol((input_buffer+1), NULL, 10);
         if (((res > 199) && (res < 411)) || (res == 0)) { // Between 200 and 410
             servo_val1 = res;
         }
     } else {
         return -EFAULT;
     }

    return count;
}

