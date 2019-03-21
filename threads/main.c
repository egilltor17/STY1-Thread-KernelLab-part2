#include "buffer.h"


int main(int argc, char **argv)
{
    // ## Register the signal handler for SIGINT
    Signal(SIGINT, sigint_handler);

    /* Seed random  */
    srand(time(NULL));
    /* Parse the input arguments from argv */
    int arg1 = 1;
    int arg2 = 1;
    int arg3 = 10;
    int arg4 = 1;
    switch (argc) {
        default:
        case 5:
            arg4 = atoi(argv[4]);
        case 4:
            arg3 = atoi(argv[3]);
        case 3:
            arg2 = atoi(argv[2]);
            if( arg2 > (2*arg1) ) {
                // ## DO NOT CHANGE THIS ###################################
                // ## If you want more consumers you will have to spawn them
                // ## some other way. Perhaps using signals from the kmt ???
                printf("Can only have 2x consumers from command line\n");
                arg2 = arg1*2;
                sleep(1);
            }
        case 2:
            arg1 = atoi(argv[1]);
        case 1:
            break;
    }
    fprintf( stdout, "Pros: %d Cons:%d Buffer:%d ??:%d\n", 
             arg1, arg2, arg3, arg4 );
    fflush( stdout );

/* Create an array of all the thread identifiers *************************/
    int num_threads = arg1 + arg2;
    int curr_thread = 0;
    pthread_t *threads = (pthread_t*) malloc( sizeof( pthread_t ) * num_threads );

/* Initialize variables in crossing.c and traffic.c **********************/
    // THIS CALLS YOUR IMPLEMENTATION OF init();
    buffer_init( arg3 );

/* Spawn the threads for producers and consumers *************************/
    int num_producers = arg1;
    int num_consumers = arg2;
    int i = 0;
    int j = 1; // 1-based numberin of the threads, used in the thread info.
    // spawn the producers 
    for (;i<num_producers; ++i) {
        // ## Allocate memory on the HEAP for the thread info. 
        thread_info *thread_data = malloc( sizeof(thread_info) );
        thread_data->thread_nr = j;
        pthread_t tid = spawn_producer( thread_data );
        threads[curr_thread++] = tid;
        j++;
    }
    i = 0;
    for (;i<num_consumers; ++i) {
        // ## Allocate memory on the HEAP for the thread info 
        thread_info *thread_data = malloc( sizeof(thread_info) );
        thread_data->thread_nr = j;
        pthread_t tid = spawn_consumer( thread_data );
        threads[curr_thread++] = tid;
        j++;
    } // end for
    // ## if we detached we can call pthread_exit(0), else we have to join.
    curr_thread--;
    for (; curr_thread >= 0; curr_thread--) {
        if (threads[curr_thread] > 0) {
            Pthread_join( threads[curr_thread], NULL );
        }
    }
    // ## DO NOT REMOVE THE CALL TO print_production_consumptions_state() ###
    print_production_consumptions_state();
    // ## Clean up the malloc..
    free ( threads );

    buffer_exit(); 
    return 0;
}
