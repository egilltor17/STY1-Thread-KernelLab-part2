#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <asm/uaccess.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
/* YOU MUST PUT YOUR NAME IN THE AUTHOR INFO */ 
MODULE_AUTHOR("egilltor17_ernir17_hallgrimura17");
/* YOU MUST ADD A DESCRIPTION OF YOUR MODULE */
MODULE_DESCRIPTION("Kernel module to make a RaspberryPi light up LEDs based on a buffer's capacity");
MODULE_VERSION("0.1");

/* BCM pinout: This is how you should connect the leds to the Pi. 
  CP2401:  
    Our SERIAL to USB hat 
    is on the first 5 pins      GREEN
   ---------------------          v 
  | 5v  5v Gnd p14 p15 | p18 Gnd p23 ....    40
1 | 3v p02 p03 p04 Gnd | p17 p27 p22 ....  
   ---------------------      ^   ^
                             RED BLUE
*/      
#define R 27 // GPIO pin of red on 3-led
#define G 23 // GPIO pin of green on 3-led
#define B 22 // GPIO pin of blue on 3-led

#define ON  1 // readability macro
#define OFF 0 // readability macro

int led_init_ok = 0;
int c; // used for error checking.

struct regsig_data {
  int pid;
  int sig;
  char pin;
  int on;
};

struct list_element {
    struct regsig_data  *data;  /* the payload */
    struct list_element *next;  /* pointer to the next element */
};

struct list_element *head = NULL;


void myKill(int pid, int sig) {
  struct task_struct* ts = NULL;
  struct pid* ps = NULL;
  rcu_read_lock();
  ps = find_get_pid(pid);
  if (ps == NULL) {
    printk(KERN_INFO "kmt dbg ps is NULL!! \n");
    rcu_read_unlock();
    return;
  } 
  ts = pid_task( ps, PIDTYPE_PID);
  if (ts == NULL) {
    printk(KERN_INFO "kmt dbg ts is NULL!! \n");
    rcu_read_unlock();
    return;
  } 
  printk(KERN_INFO "kmt dbg KILL! KILL! KILL! \n");
  if ( send_sig_info(sig, SEND_SIG_NOINFO, ts) < 0) {
    printk(KERN_INFO "kmt dbg send_sig_info() returned less than zero :( \n");
  }
  printk(KERN_INFO "kmt dbg Signal SENT, Are you dead yet??? \n");
  rcu_read_unlock();
}


// ## functions for the kernel module
//static int __init init_light( void )
static int init_light( void ) {
  // request pins for R, G, B
  printk(KERN_INFO "kmt Requsting the R, G, B GPIO pins for light..\n");
  c = 0;

  /***********************************************************
   * HERE YOU HAVE TO REQUEST EACH GPIO PIN YOU WISH TO USE  *
   * USE THE gpio_request() FUNCTION FOR THIS.               *
   * int gpio_request(unsigned int gpio, const char *label); *
   ***********************************************************/
  
  c += gpio_request(R, "red");
  c += gpio_request(G, "green");
  c += gpio_request(B, "blue");
  
  if (c != 0) {
    printk(KERN_INFO "kmt Requsting of one or more pins has failed\n");
    return -1;
  }

  printk(KERN_INFO "kmt Setting Direction of R, G, B pins..\n");
  /***********************************************************
   * HERE YOU HAVE TO SET EACH PINS DIRECTION (IN OR OUT)    *
   * USE THE gpio_direction_output() FUNCTION FOR THIS.      *
   * int gpio_direction_input(unsigned int gpio);            *
   * int gpio_direction_output(unsigned int gpio, int value);*
   ***********************************************************/

  c += gpio_direction_output(R, ON);
  c += gpio_direction_output(G, ON);
  c += gpio_direction_output(B, ON);
  
  if (c != 0) {
    printk(KERN_INFO "kmt Setting Directon of one or more pins has failed\n");
    return -2;
  }

  /***********************************************************
   * HINT: mdelay() and gpio_set_value()                     *
   ***********************************************************/
  
  printk(KERN_INFO "kmt Waiting 2 seconds then turning defaulting to lights off.\n");
  
  mdelay(2000);
  gpio_set_value(R, OFF);
  gpio_set_value(G, OFF);
  gpio_set_value(B, OFF);
  
  printk(KERN_INFO "kmt light setup completed successfully.\n");
  led_init_ok = 1; 
  return 0;
}

//static void __exit exit_light ( void ) {
static void exit_light ( void ) {
  
  /***********************************************************
   * HERE YOU HAVE TO DO ALL CLEAN NEEDED ON MODULE REMOVAL  *
   * REMEMBER THAT YOU WILL NEED TO RELEASE THE GPIO PINS    *
   ***********************************************************/

  /* turn off all the lights before freeing the pins */ 
  gpio_set_value(R, OFF);
  gpio_set_value(G, OFF);
  gpio_set_value(B, OFF);
  gpio_free(R);
  gpio_free(G);
  gpio_free(B);
  printk(KERN_INFO "Turning off the lights\n");
  return;
}

// ## Here is the function for writing to the /sys/light/light file.
static ssize_t light_store( struct kobject* kobj, struct kobj_attribute *attr, const char* buf, size_t count) {
  /***********************************************************
   * THIS IS THE FUNCTION THAT RUNS WHEN DATA IS WRITTEN TO  *
   * YOUR /sys/light/light FILE.                             *
   * THE DATA IS IN THE CHARACHTER ARRAY buf AND count TELLS *
   * YOU HOW LONG THE ARRAY IS. YOU WILL NEED TO PARSE THE   *
   * INPUT AND SET THE LIGHTS ACCORDINGLY.                   *
   ***********************************************************/

  /***********************************************************
   * You must add functionality to the light_store()         *
   * function such that signals are sent to those processes  *
   * that have registered to be notified about the given     *
   * change. For example if process with pid 9999 has asked  *
   * to be sent signal 10 (SIGUSR1) when the red light is    *
   * turned on then, when light_store changes the red light  *
   * from 0 to 1 a signal should be sent (but not if it was  *
   * already 1, only if it was chanted from 0 to 1).         *
   ***********************************************************/ 
  struct list_element *runner = head;
  int r, g, b;
  /* Validate the input before trying to read from buf */
  if(count != 6 || buf[1] != ' ' || buf[3] != ' ' || buf[5] != '\n') {
    printk(KERN_INFO "light buf has wrong input format\n");
    return -2;
  }
  r = buf[0] == '1';
  g = buf[2] == '1';
  b = buf[4] == '1';
  while(runner != NULL) {
    if(runner->data->pin == 'R' && (runner->data->on ^ gpio_get_value(R))) {
      myKill(runner->data->pin, runner->data->sig);  
    } else if(runner->data->pin == 'G' && (runner->data->on ^ gpio_get_value(G))) {
      myKill(runner->data->pin, runner->data->sig);  
    } else if(runner->data->pin == 'B' && (runner->data->on ^ gpio_get_value(B))) {
      myKill(runner->data->pin, runner->data->sig);  
    } 
    runner = runner->next;
  }

  /* turns the LEDs on if the buf has 1's at the correct indices, off otherwise */
  gpio_set_value(R, r);
  gpio_set_value(G, g);
  gpio_set_value(B, b);

  return count;
}

// ## Here is the function for reading the /sys/light/light file.
static ssize_t light_show( struct kobject *kobj, struct kobj_attribute *attr, char* buf) {
  /***********************************************************
   * THIS IS THE FUNCTION THAT RUNS WHEN DATA IS READ FROM   *
   * YOUR /sys/light/light FILE.                             *
   * THE DATA IS NOT RETURNED BUT RATHER IT IS PUT IN THE    *
   * CHARACHTER ARRAY buf IN THE FORM "R G B\N", WHERE EACH  *
   * LETTER IS EITHER 0 OR 1.                                *
   * THE RETURN VALUE SHOULD BE THE LENGTH OF THE STRING.    *
   ***********************************************************/

  /* The input is a string of the format "R G B\n" where each letter is either a 0 or 1 */
  buf[0] = gpio_get_value(R);
  buf[1] = ' ';
  buf[2] = gpio_get_value(G);
  buf[3] = ' ';
  buf[4] = gpio_get_value(B);
  buf[5] = '\n';

  // THE RETURN VALUE SHOULD BE THE LENGTH OF THE STRING IN buf.
  return 6;
}

/***************************************************
 *    XXXX    XXXXX   XXXX    XXXX   X    XXXX    *
 *    X   X   X      X       X       X   X        * 
 *    XXXX    XXX    X  XX    XXX    X   X  XX    *
 *    X   X   X      X   X       X   X   X   X    *
 *    X   X   XXXXX   XXXX   XXXX    X    XXXX    *
 ***************************************************/

static void exit_regsig ( void ) {
  /***********************************************************
   * HERE YOU HAVE TO DO ALL CLEAN NEEDED ON MODULE REMOVAL  *
   * REMEMBER THAT YOU WILL NEED TO RELEASE THE GPIO PINS    *
   ***********************************************************/
  struct list_element *prev = head;
  
  while(head != NULL){
    head = head->next;
    kfree(prev->data);
    kfree(prev);
    prev = head;
  }
  
  return;
}

// ## Here is the function for writing to the /sys/regsig/regsig file.
static ssize_t regsig_store( struct kobject* kobj, struct kobj_attribute *attr, const char* buf, size_t count) {
  /***********************************************************
   * Implement the reqsig_store() functions such that can    *
   * pars from the input (i.e. from buf) the 4 parameters    *
   * that are needed:                                        *
   *      I)   the pid of the process                        *
   *      II)  the number of the signal to send to           *
   *      III) a led (pin) to monitor (R/G/B)                *
   *      IV)  the action to look for (i.e. turning led on   *
   *           or off)                                       *
   * You must create some data structure to store this info  *
   * You should not register the same information multiple   *
   * times. I.e. if an identical registration comes in it    *
   * should be ignored.                                      *
   ***********************************************************/
  /* Data: PPPPP SS RGB O\n    -    pid, signal, RGB, ON/OFF */

  struct list_element *node;
  struct list_element *prev;
  struct list_element *runner;
  
  /* The data partitions must be seperated by a space */
  /*   length            pid               signal           RGB              ON/OFF     */
  if(count != 13 || buf[5] != ' ' || buf[8] != ' ' || buf[10] != ' ' || buf[12] != '\n') {
    printk(KERN_INFO "regsig buf has wrong input format\n");
    return -2;
  }
  if(buf[9] != 'R' || buf[9] != 'G' || buf[9] != 'B') {
    printk(KERN_INFO "regsig buf's pin is invalid, must be 'R', 'G' or 'B'\n");
    return -1;
  } 

  node = kmalloc(sizeof(struct list_element), GFP_KERNEL);
  node->data = kmalloc(sizeof(struct regsig_data), GFP_KERNEL);
  node->next = NULL;

  if(sscanf(buf, "%d %d %c %d\n", &(node->data->pid), &(node->data->sig), &(node->data->pin), &(node->data->on)) != 4) {
    printk(KERN_INFO "regsig buf has wrong input format. buf: %s\n", buf);
    return -2;
  }
  
  if(head == NULL) {
    head = node;
    head->next = NULL;
    return count;
  }
  if(head->data->pid <= node->data->pid) {
    if(head->data->pid == node->data->pid) {
      kfree(head->data);
      head->data = node->data;
      kfree(node);
    } else {
      head->next = node;
      head = node;
    }
    return count;
  }

  prev = head;
  runner = head->next;
  while(runner != NULL && runner->data->pid < node->data->pid) {
    prev = runner;
    runner = runner->next;
  }

  if(runner->data->pid == node->data->pid) {  /* we replace runner's data with node's */
    kfree(runner->data);
    runner->data = node->data;
    kfree(node);
  } else {                                    /* we insert the node between prew and runner */
    prev->next = node;
    node->next = runner;
  }
  return count;
}

// ## Here is the function for reading the /sys/light/light file.
static ssize_t regsig_show( struct kobject *kobj, struct kobj_attribute *attr, char* buf) {
  int size;
  struct list_element *runner = head;
  
  /***********************************************************
   * You must implement the reqsig_show() function that      *
   * prints out all currently registrations in order of pid. *
   ***********************************************************/

  size = 0;
  while(runner != NULL) {
    size += sprintf(buf+size, "%d %d %c %d\n", 
      runner->data->pid, runner->data->sig, runner->data->pin, runner->data->on);
    runner = runner->next;
  }

  // THE RETURN VALUE SHOULD BE THE LENGTH OF THE STRING IN buf.
  return size;
}

// ## sysfs variables ## 
// This must come after the function declarations as they are used here.
// We need a kernel object (kobject) and to configur the attributes for the file
static struct kobject* light_kobj;
// static struct kobject* regsig_kobj;
// ### Warning! We need write-all permission so overriding check, THIS IS NOT RECOMENDED! 
#undef VERIFY_OCTAL_PERMISSIONS
#define VERIFY_OCTAL_PERMISSIONS(perms) (perms)
static const struct kobj_attribute light_attr = __ATTR( light, 0666, light_show, light_store );
static const struct kobj_attribute regsig_attr = __ATTR( regsig, 0666, regsig_show, regsig_store );

// ## Kernel module functions 
// we must register a directory light in /sys/ and make a file /sys/light/light
static int kmt_sysfs_init( void ) {
  printk(KERN_INFO "kmt Starting sysfs...\n");

  /***********************************************************
   * HERE YOU MUST CREATE THE KERNEL OBJECT (light_kobj) BY  *
   * CALLING THE kobject_create_and_add() FUNCTION:          *
   * struct kobject* kobject_create_and_add(                 *
   *                               const char * name,        *
   *                               struct kobject * parent); *
   * IF THE CALL FAILS A NULL VALUE IS RETURNED.             *
   *                                                         *
   * YOU WILL THEN HAVE TO CREATE THE sysfs FILE USING THE   *
   * int sysfs_create_file(struct kobject * kobj,            *
   *                       const struct attribute * attr);   *
   *                                                         *
   * NOTE: THE ATTRIBUTES ARE ALREADY CREATED IN line 143    *
   * FOR MORE INFO:                                          *
   * www.kernel.org/doc/Documentation/filesystems/sysfs.txt  *
   ***********************************************************/

  /* need to catch errors, when creating kernel objects */  
  if ((light_kobj = kobject_create_and_add("light", NULL)) == NULL) {
    printk(KERN_INFO "kobject_create_and_add(light) has failed\n");
    return -2;
  }
  
  /* light =========================================================== */
  if (sysfs_create_file(light_kobj, &light_attr.attr) != 0) {
    printk(KERN_INFO "sysfs_create_file(light) has failed\n");
    return -2;
  }

  printk(KERN_INFO "kmt Finished light sysfs setup.\n");
  
  /* regsig ========================================================== */
  if (sysfs_create_file(light_kobj, &regsig_attr.attr) != 0) {
    printk(KERN_INFO "sysfs_create_file(regsig) has failed\n");
    return -2;
  }

  printk(KERN_INFO "kmt Finished regsig sysfs setup.\n");
  return 0;
}

static void kmt_sysfs_exit( void ) {
  printk(KERN_INFO "kmt sysfs exiting....\n");
  kobject_put( light_kobj );  // clean up the light kobject upon exit
  printk(KERN_INFO "kmt sysfs unloaded.\n");
  return;
}

static int __init init_kmt( void ) {
  int ret = 0;
  ret = init_light( );
  // ret = init_regsig( );  /* ToDo? */
  ret = kmt_sysfs_init( );
  return ret;
}
static void __exit exit_kmt ( void ) {
  kmt_sysfs_exit( );
  exit_light( );
  exit_regsig( );
}

module_init(init_kmt);
module_exit(exit_kmt);
