#ifndef BUFFER_H
#define BUFFER_H
#include "csapp.h"

#define max_sleep_time 2000

struct thread_info {
    int thread_nr;
} typedef thread_info;

// ## Variables needed for the buffer logic. #################//
unsigned int * buff;
int first_slot;
int last_slot;
int free_slots;
int num_slots;

// ## Halting condition for threads ##########################//
int producers_run;
int consumers_run;

// ## Declaration of signal handler functions ################//
void sigint_handler (int sig);

// ## Random sleep function in milliseconds ##################//
long rand_sleep(int ms);
unsigned int get_rand_int_from_file();

// ## Initilization and destruction of buffer ################//
void buffer_init(unsigned int);
void buffer_exit(void);

// ## Thread functions #######################################//
void* producer( void* vargp  );
void* consumer( void* vargp  );

// ## Variables needed for the resturant #####################//
int  entree_produced;
int  entree_consumed;
int  steaks_produced;
int  steaks_consumed;
int  vegan_produced;
int  vegan_consumed;
int  dessert_produced;
int  dessert_consumed;
void print_production_consumptions_state();

// ## Work functions #########################################//
struct timeval* produce(unsigned int* i);
struct timeval* consume(unsigned int i);
int  produce_entree();
int  produce_steak();
int  produce_vegan();
int  produce_dessert();
int  consume_entree();
int  consume_steak();
int  consume_vegan();
int  consume_dessert();

// ## Declaration of locking mechanisms (semaphors) ##########//
sem_t producers; // producer part of conumer/producer algo.
sem_t consumers; // consumer part of conumer/producer algo.
sem_t last_slot_lock; // protects the buffer and last_slot index
sem_t free_slot_lock; // protects the buffer and free_slot index
/******************************************************
 * MISSING CODE 1/6                                   *
 * We added the locks for the ringbuffer but you may  *
 * need more locks, right?                            *
 ******************************************************/


// ## Functions to create new threads ########################//
pthread_t spawn_producer(thread_info *arg);
pthread_t spawn_consumer(thread_info *arg);

#endif /* BUFFER_H */
