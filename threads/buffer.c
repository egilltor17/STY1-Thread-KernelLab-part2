#include "buffer.h"

#define LIGHTFILE "/sys/light/light"
FILE* light;
#define RANDFILE "/dev/random"
FILE* devrand;


// ## Signal handler to handle SIGINT (Ctrl-C)
void sigint_handler (int sig) {
    printf("\n\nCtrl-C was caught\n");
    printf("Halting all threads..\n\n");
    producers_run = 0;
    consumers_run = 0;
}

void print_production_consumptions_state() {
    // char string [200];
    char string [800];
	P(&sem_print);
    	sprintf(string, "Entrees: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\nSteaks: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\nVegan: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\nDesserts: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\n", entree_produced, entree_consumed, steaks_produced, steaks_consumed, vegan_produced, vegan_consumed, dessert_produced, dessert_consumed);
	V(&sem_print);
    /* 
	sprintf(string, "Entrees: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\n", entree_produced, entree_consumed);
    Sio_puts(string)
    sprintf(string, "Steaks: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\n", steaks_produced, steaks_consumed);
    Sio_puts(string);
    sprintf(string, "Vegan: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\n", vegan_produced, vegan_consumed);
    Sio_puts(string);
    sprintf(string, "Desserts: \n\tREADY(produced)= %d \n\tSOLD(consumed)=%d\n", dessert_produced, dessert_consumed); 
	*/
    Sio_puts(string);
}

// ## Random sleep functions used in work functions.
long rand_sleep(int ms) {
    long sleep_factor = rand() % 1000;
    long ret = (ms * 1000) + (sleep_factor * 1000);
    usleep( ret );
    return ret;
}

// ## Read 4 random chars from RANDFILE and makes an int
unsigned int get_rand_int_from_file() {
    unsigned int ret;
    fread(&ret, 1, 4, devrand);
    return ret;
}

// ## INITILIZATION OF the RINGBUFFER ########################//
void buffer_init(unsigned int buffersize) {
    Sio_puts("initializing the circular buffer!\n");
    // ## Set the two thread run consditions 
    producers_run = 1; // while true producers loop.
    consumers_run = 1; // while true consumers loop.
    
    // ## Allocating the ringbuffer and initializing variabls. 
    num_slots = buffersize; // size of the buffer in slots.
    buff = (unsigned int*) malloc( num_slots*sizeof(unsigned int) );
    first_slot = 0;         // the front of circle
    last_slot = 0;          // the end of the circle
    free_slots = num_slots; // the number of empty slots
    
    entree_produced  = 0;
    entree_consumed  = 0;
    steaks_produced  = 0;
    steaks_consumed  = 0;
    vegan_produced   = 0;
    vegan_consumed   = 0;
    dessert_produced = 0;
    dessert_consumed = 0;
    
    // ## Initializing the thread-locking mechanisms 
    /******************************************************
     * MISSING CODE 2/6                                   *
     *                                                    *
     * NOTE!!! YOU MUST FIRST CREATE THE SEMAPHORES       *
     * IN buffer.h                                        *
     ******************************************************/
    Sem_init(&sem_producers, 0, buffersize);	/* empty slots 	- producer part of consumer/producer algo. */
    Sem_init(&sem_consumers, 0, 0);           	/* full slots  	- consumer part of consumer/producer algo. */
    Sem_init(&slot_lock, 0, 1);      			/* mutex       	- protects the buffer indexes  */

	Sem_init(&sem_spoon, 0, 2);					/* nr of spoons	- protects the number of avalible spoons */
	Sem_init(&sem_time, 0, 1);					/* mutex	   	- protects the prosses time */
	Sem_init(&sem_print, 0, 1);					/* mutex	   	- protects the print statements */

	Sem_init(&sem_entree_produced, 0, 1);		/* mutex       	- protects the entree_produced  counter */
	Sem_init(&sem_entree_consumed, 0, 1);		/* mutex       	- protects the entree_consumed  counter */
	Sem_init(&sem_steaks_produced, 0, 1);		/* mutex       	- protects the steaks_produced  counter */
	Sem_init(&sem_steaks_consumed, 0, 1);		/* mutex       	- protects the steaks_consumed  counter */
	Sem_init(&sem_vegan_produced, 0, 1);		/* mutex       	- protects the vegan_produced   counter */
	Sem_init(&sem_vegan_consumed, 0, 1);		/* mutex       	- protects the vegan_consumed   counter */
	Sem_init(&sem_dessert_produced, 0, 1);		/* mutex       	- protects the dessert_produced counter */
	Sem_init(&sem_dessert_consumed, 0, 1);		/* mutex       	- protects the dessert_consumed counter */

    // ## Try to open the /sys/light/light file.
    if( (light = fopen(LIGHTFILE, "r+")) == NULL) { 
        // failed and thus we open a local directory file instead.
         if ( (light = fopen( "./light", "w+")) == NULL) {
              printf("Failed to open the light file :( \n");
              exit(-1);
      	 }
    }
    // ## Try to open the /dev/random file.
    if( (devrand = fopen(RANDFILE, "r")) == NULL) { 
        Sio_puts("Failed to open the light file :( \n");
        exit(-1);
    } else {
        printf("/dev/random test: %d \n", get_rand_int_from_file());
    } 
    // As the buffer is empty we start with a green light.
    //             "R G B\n".
    fprintf(light, "0 1 0\n");
    fflush(light); // we must not buffer this output.
}

// ## DESTRUCTION OF the RINGBUFFER ########################//
void buffer_exit(void) {
    Sio_puts("\n\n\nThis party is over!!\n");
    rand_sleep(2000);
    Sio_puts("Turning off the lights.\n");
    fprintf(light, "0 0 0\n");
    fflush(light);
    Sio_puts("So long and thanks for all the fish!\n\n");
    fclose(light);
    free(buff);
}

// PART 2 TASK A)
// #########################################################//
// ## NEED TO MAKE THESE FUNCTIONS THREAD-SAFE BUT YOU MAY #//
// ## NOT CHANGE WHAT THEY ESSENTIALLY DO. YOU CAN HOWEVER #//
// ## RE-WRITE THEM TO DO IT BETTER AND/OR THREAD-SAFE     #//
// ## I.E. YOU MAY CHANGE PRINTF TO SIOPUT BUT NOT REMOVE  #//
// ## NOR SHORTEN THE CALLS TO rand_sleep() NOR SKIP OUTPUT#//
// #########################################################//
// ## The work functions for producers #####################//
int produce_entree() {
    rand_sleep(100);
	P(&sem_entree_produced);
    	entree_produced++;
	V(&sem_entree_produced);
    return 0;
}
int produce_steak() {
    rand_sleep(100);
    P(&sem_steaks_produced);
		steaks_produced++;
    V(&sem_steaks_produced);
    return 0;
}
int produce_vegan() {
    rand_sleep(100);
    system("./micro.sh");
    P(&sem_vegan_produced);
		vegan_produced++;
    V(&sem_vegan_produced);
    return 0;
}
int produce_dessert() {
    rand_sleep(100);
    Sio_puts("         _.-.         \n       ,'/ //\\       \n      /// // /)       \n     /// // //|       \n    /// // ///        \n   /// // ///         \n  (`: // ///          \n   `;`: ///           \n   / /:`:/            \n  / /  `'             \n / /                  \n(_/  hh               \n");    
	P(&sem_dessert_produced);
		dessert_produced++;
	V(&sem_dessert_produced);
    return 0;
}
struct timeval* produce(unsigned int* i) {
    // ## start & end are used to time spent
    struct timeval start; 
    struct timeval end;
    struct timeval* ret = (struct timeval*) malloc( sizeof(struct timeval) );
    gettimeofday(&start, NULL);
    unsigned int x = get_rand_int_from_file() % 4;
    switch (x) {
      case 0: 
        Sio_puts("Making E & V cos x is 0\n");
        produce_entree(); // entree and vegan dish
        produce_vegan();
        break;
      case 1:
        Sio_puts("Making V & D cos x is 1\n");
        produce_vegan(); // vegan and dessert
        produce_dessert();
        break;
      case 2:
        Sio_puts("Steak Only! cos x is 2\n");
        produce_steak(); // just the steak
        break;
      case 3 :
        Sio_puts("Steak Menu! cos x is 3\n");
        produce_entree(); // 3 course steak dinner
        produce_steak();
        produce_dessert();
    } // end swtich 
    gettimeofday(&end, NULL);
    timersub(&end, &start, ret);
    // set the return value for the buffer 
    *i = x;
    return ret;
}
// PART 2 TASK B) 
// #########################################################//
// ## NEED TO MAKE THESE FUNCTIONS THREAD-SAFE BUT YOU MAY #//
// ## NOT CHANGE WHAT THEY ESSENTIALLY DO. YOU CAN CHANGE/ #//
// ## RE-WRITE THEM TO DO "IT" BETTER AND/OR THREAD-SAFE   #//
// ## I.E. YOU MAY CHANGE PRINTF TO SIOPUT BUT NOT REMOVE  #//
// ## NOR SHORTEN THE CALLS TO rand_sleep() NOR SKIP OUTPUT#//
// #########################################################//
// ## The work functions for consumers #####################//
int consume_entree(){
    if (entree_produced < 1) {
        // if this happens then something bad is going on :/
        Sio_puts("WHO STOLE MY ENTREE!!!!\n");
        // ## PENALTY FOR LOOSING AN ORDER !!! YOU MAY NOT CHANGE THIS #
        rand_sleep(10000);
        return -1;
    } else {
		P(&sem_entree_produced);
        	entree_produced--;
		V(&sem_entree_produced);
        rand_sleep(1000);
		P(&sem_entree_consumed);
        	entree_consumed++;
		V(&sem_entree_consumed);
    }
    return 0;
}

int consume_steak(){
    if (steaks_produced < 1) {
        // ## if this happens then something bad is going on :/
        Sio_puts("STEAK THEAF !\n");
        // ## PENALTY FOR LOOSING AN ORDER !!! YOU MAY NOT CHANGE THIS #
        rand_sleep(10000);
        return -1;
    } else {
		P(&sem_steaks_produced);
        	steaks_produced--;
		V(&sem_steaks_produced);
        rand_sleep(3000);
        P(&sem_steaks_consumed);
			steaks_consumed++;
        V(&sem_steaks_consumed);
    }
    return 0;
}

int consume_vegan() {
    if (vegan_produced < 1) {
        // ## if this happens then something bad is going on :/
        Sio_puts("WHO STEALS A VEGAN DISH?! \n");
        // ## PENALTY FOR LOOSING AN ORDER !!! YOU MAY NOT CHANGE THIS #
        rand_sleep(10000);
        return -1;
    } else {
		P(&sem_vegan_produced);
        	vegan_produced--;
		V(&sem_vegan_produced);
        rand_sleep(500);
        system("./munch.sh");
        rand_sleep(500);
        P(&sem_vegan_consumed);
			vegan_consumed++;
        V(&sem_vegan_consumed);
    }
    return 0;
}

int consume_dessert() {
    // ## The resturant only has two spoons :( Ppl. will have to share!
    // #################################################################
    // static int spoon = 2; // ## YOU MAY NOT CHANGE THIS!! ##############
    // #################################################################
    if (dessert_produced < 1) {
        // ## if this happens then something bad is going on :/
        Sio_puts("I SCREAM FOR ICE-CREAM?!\n");
        // ## PENALTY FOR LOOSING AN ORDER !!! YOU MAY NOT CHANGE THIS #
        rand_sleep(10000);
        return -1;
    } else {
        // ## wait for the a spoon.. (is this the best way to do this??)
		P(&sem_spoon);
		// 	spoon--;
		// V(&sem_spoon);
		P(&sem_dessert_produced);
			dessert_produced--;
		V(&sem_dessert_produced);
        rand_sleep(600);
		P(&sem_dessert_consumed);
			dessert_consumed++;
		V(&sem_dessert_consumed);
		// P(&sem_spoon);
		// 	spoon++;
		V(&sem_spoon);
    }
    return 0;   
}

// ## The work function for consumers #####################//
struct timeval* consume(unsigned int i) {
    // ## start & end are used to time spent
    struct timeval start; 
    struct timeval end;
    struct timeval* ret = (struct timeval*) malloc( sizeof(struct timeval) );
    gettimeofday(&start, NULL);
	switch (i) {
		case 0:
			Sio_puts("I must have orderd E & V cos i is 0\n"); 
			consume_entree(); // entree and vegan dish
			consume_vegan();
			break;
		case 1:
			Sio_puts("Who orderd this stuff(V & E)? ME? 1\n");
			consume_vegan(); // vegan and dessert
			consume_dessert();
			break;
		case 2:
			Sio_puts("MEEEEEET! (Steak Only) cos i is 2\n");
			consume_steak(); // just the steak
			break;
		case 3 :
			Sio_puts("Steak Menu! Ví, Ví, ce moi! cos i is 3\n");
			consume_entree(); // 3 course steak dinner
			consume_steak();
			consume_dessert();
	} // end swtich 
    gettimeofday(&end, NULL);
    timersub(&end, &start, ret);
    return ret;
}

// ## The main function for producer threads #############// 
void* producer( void* vargp ) {
    // ## used to calculate how long the thread has run.
    struct timeval thrd_runtime;
    timerclear(&thrd_runtime);

    while(producers_run) {
        if ( !free_slots ) {
            Sio_puts("The buffer is full :( \n");
            // As the buffer is full set the red light.
            //             "R G B\n".
            fprintf(light, "1 0 0\n");
            fflush(light);
            // break;
        } else {
            // Neither full nor empty so we show blue/yellow.
            //             "R G B\n".
            fprintf(light, "0 0 1\n");
            fflush(light);
        }

        /******************************************************
         * MISSING CODE 3/6                                   *
         * HERE YOU MUST REVISE AND ADD YOUR CODE FROM PART 1 *
         ******************************************************/
        
		// ## if there is a free slot we produce to fill it.
        P(&sem_producers);

			// ## produce() takes reference to the product to produce. 
			unsigned int prod = 0;
			// ## returns a timeval struct that was malloced. 
			struct timeval* t = produce(&prod);
			// ## add to the thread running time total and free t.
			P(&sem_time);
				timeradd(&thrd_runtime, t, &thrd_runtime);
			V(&sem_time);
			free(t); // if you DELETE ME you will have a MEMORY LEEK!!! 

			P(&slot_lock);
				// ## update add produced value (called prod) to the array.
				int slot = last_slot++;  	// filled a slot so move index
				if ( last_slot == num_slots ) {
					last_slot = 0;         	// we must not go out-of-bounds.
				}
				free_slots++; 				// one less free slots available
			V(&slot_lock);
			
			P(&sem_print);
				printf("Putting production %u in slot %d\n", prod, slot);
			V(&sem_print);
			buff[slot] = prod;
        V(&sem_consumers);
    } // end while
    printf("Thread Runningtime was ~%lusec. \n", thrd_runtime.tv_sec);

    // this is just for debugging, you may remove this.   
    // print_production_consumptions_state();

    return NULL;
}

// ## The main function for consumer threads #############// 
void* consumer( void* vargp ) {

    // ## used to calculate how long the thread has run.
    struct timeval thrd_runtime;
    timerclear(&thrd_runtime);
    while (consumers_run) {
        if (num_slots - free_slots == 0){
            printf("The buffer is empty :( \n");
            // As the buffer is empty we start with a green light 
            //             "R G B\n".
            fprintf(light, "0 1 0\n");
            fflush(light);
            // break;
        } else {
            // Neither full nor empty so we show blue/yellow.
            //             "R G B\n".
            fprintf(light, "0 0 1\n");
            fflush(light);
        }

        /******************************************************
         * MISSING CODE 4/6                                   *
         * HERE YOU MUST REVISE AND ADD YOUR CODE FROM PART 1 *
         ******************************************************/     

        P(&sem_consumers);			
			P(&slot_lock);
				int slot = first_slot++;	// update buff index.
				if (first_slot == num_slots ) {
					first_slot = 0;         // we must not go out-of-bounds.
				}
				free_slots++;      			// one more free slots available
			V(&slot_lock);
			
			int tmp_prod = buff[slot];
			buff[slot] = -1;            	// zero the slot consumed.
			struct timeval* t = consume(tmp_prod);
			
			P(&sem_print);
				printf("Consumer takes prod from slot %d and consumes prod %d \n", slot, tmp_prod);
			V(&sem_print);

			P(&sem_time);
				timeradd(&thrd_runtime, t, &thrd_runtime);
			V(&sem_time);

			free(t); // ef you DELETE ME you will have a MEMORY LEEK!!!     
        
        V(&sem_producers);
        
    } // end while
    printf("Thread Runningtime was ~%lusec. \n", thrd_runtime.tv_sec);

    // this is just for debugging, you may remove this. 
    // print_production_consumptions_state();

    return NULL;
}

pthread_t spawn_producer( thread_info *arg )
{
    printf("Spawning thread %d as a producer \n", arg->thread_nr);
  
    // producer(NULL);
    /******************************************************
     * MISSING CODE 5/6                                   *
     * HERE YOU MUST REVISE AND ADD YOUR CODE FROM PART 1 *
     ******************************************************/
    pthread_t tid;
    Pthread_create(&tid, NULL, producer, NULL);
    return tid;
}

pthread_t spawn_consumer( thread_info *arg )
{
    printf("Spawning thread %d as a consumer\n", arg->thread_nr);

    // consumer(NULL);
    /******************************************************
     * MISSING CODE 6/6                                   *
     * HERE YOU MUST REVISE AND ADD YOUR CODE FROM PART 1 *
     ******************************************************/
    pthread_t tid;
    Pthread_create(&tid, NULL, consumer, NULL);
    return tid;
}
