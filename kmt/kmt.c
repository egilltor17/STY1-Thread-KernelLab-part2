#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

MODULE_LICENSE("GPL");
/* YOU MUST PUT YOUR NAME IN THE AUTHOR INFO */ 
/* missing code  1/9 */ 
MODULE_AUTHOR("egilltor17_ernir17_hallgrimura17");
/* YOU MUST ADD A DESCRIPTION OF YOUR MODULE */
/* missing code 2/9 */ 
MODULE_DESCRIPTION("Kernel module to make a RaspberryPi light up LEDs based on a buffer's capasity");
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
  
  /* missing code 3/9 */ 
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

  /* missing code 4/9 */ 
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
  /* missing code 5/9 */ 
  
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

  /* missing code 6/9 
   * turn off all the lights before freeing the pins
   */ 
  gpio_set_value(R, OFF);
  gpio_set_value(G, OFF);
  gpio_set_value(B, OFF);
  gpio_free(R);
  gpio_free(G);
  gpio_free(B);

  return;
}

// ## Here is the function for writing to the /sys/light/light file.
static ssize_t light_store( struct kobject* kobj, 
  struct kobj_attribute *attr, const char* buf, size_t count) {
  /***********************************************************
   * THIS IS THE FUNCTION THAT RUNS WHEN DATA IS WRITTEN TO  *
   * YOUR /sys/light/light FILE.                             *
   * THE DATA IS IN THE CHARACHTER ARRAY buf AND count TELLS *
   * YOU HOW LONG THE ARRAY IS. YOU WILL NEED TO PARSE THE   *
   * INPUT AND SET THE LIGHTS ACCORDINGLY.                   *
   ***********************************************************/

  /* missing code 7/9 */
  /* The input is a string of the format "R G B\n" where each letter 
   * is either a 0 or 1 
   * 
   * Validate the input before trying to read from buf  
   */
  if(count != 6 || buf[1] != ' ' || buf[3] != ' ' || buf[5] != '\n') {
    printk(KERN_INFO "buf has wrong input format\n");
    return -2;
  }
  /* turns the LEDs on if the buf has 1's at the correct indices, off otherwise */
  gpio_set_value(R, buf[0] == '1');
  gpio_set_value(G, buf[2] == '1');
  gpio_set_value(B, buf[4] == '1');

  return count;
}

// ## Here is the function for reading the /sys/light/light file.
static ssize_t light_show( struct kobject *kobj, 
  struct kobj_attribute *attr, char* buf) {
  /***********************************************************
   * THIS IS THE FUNCTION THAT RUNS WHEN DATA IS READ FROM   *
   * YOUR /sys/light/light FILE.                             *
   * THE DATA IS NOT RETURNED BUT RATHER IT IS PUT IN THE    *
   * CHARACHTER ARRAY buf IN THE FORM "R G B\N", WHERE EACH  *
   * LETTER IS EITHER 0 OR 1.                                *
   * THE RETURN VALUE SHOULD BE THE LENGTH OF THE STRING.    *
   ***********************************************************/

  /* missing code 8/9 */
  /* The input is a string of the format "R G B\n" where each letter 
   *  is either a 0 or 1 
   */

  buf[0] = gpio_get_value(R);
  buf[1] = ' ';
  buf[2] = gpio_get_value(G);
  buf[3] = ' ';
  buf[4] = gpio_get_value(B);
  buf[5] = '\n';

  // THE RETURN VALUE SHOULD BE THE LENGTH OF THE STRING IN buf.
  return 6;
}

// ## sysfs variables ## 
// This must come after the function declarations as they are used here.
// We need a kernel object (kobject) and to configur the attributes for the file
static struct kobject* light_kobj;
// ### Warning! We need write-all permission so overriding check, THIS IS NOT RECOMENDED! 
#undef VERIFY_OCTAL_PERMISSIONS
#define VERIFY_OCTAL_PERMISSIONS(perms) (perms)
static const struct kobj_attribute light_attr = __ATTR( light, 0666, light_show, light_store );

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

  
  /* missing code 9/9 */
    
  /* need to catch errors, when creating kernel objects */  
  if ((light_kobj = kobject_create_and_add("light", NULL)) == NULL) {
    printk(KERN_INFO "kobject_create_and_add has failed\n");
    return -2;
  }
  
  if (sysfs_create_file(light_kobj, &light_attr.attr) != 0) {
    printk(KERN_INFO "sysfs_create_file has failed\n");
    return -2;
  }

  printk(KERN_INFO "kmt Finished sysfs setup.\n");
  return 0;
}

static void kmt_sysfs_exit( void ) {
  printk(KERN_INFO "kmt sysfs exiting....\n");
  kobject_put( light_kobj ); // clen up the allocated kobject upon exit
  printk(KERN_INFO "kmt sysfs unloaded.\n");
  return;
}

static int __init init_kmt( void ) {
  int ret = 0;
  ret = init_light( );
  ret = kmt_sysfs_init( );
  return ret;
}
static void __exit exit_kmt ( void ) {
  kmt_sysfs_exit( );
  exit_light( );
}

module_init(init_kmt);
module_exit(exit_kmt);
