#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>


/*
*   Referenced the following students work:
*   https://github.com/cu-ecen-aeld/assignments-3-and-later-yinwenjie/blob/main/examples/threading/threading.c
*/


// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)


void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    int wait_time_us = thread_func_args->wait_to_obtain_ms * 1000;

    if(usleep(wait_time_us) != 0){
        syslog(LOG_DEBUG,"usleep wait obtain failed");
        thread_func_args -> thread_complete_success = false;
        return thread_param;
    }

    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if(rc != 0){
        syslog(LOG_DEBUG,"pthread_mutex_lock failed with %d", rc);
        thread_func_args -> thread_complete_success = false;
        return thread_param;
    }

    wait_time_us = thread_func_args->wait_to_release_ms * 1000;
    if(usleep(wait_time_us) != 0){
        syslog(LOG_DEBUG,"usleep wait release failed");
        thread_func_args -> thread_complete_success = false;
        return thread_param;
    }

    rc = pthread_mutex_unlock(thread_func_args->mutex);

    if(rc != 0){
        syslog(LOG_DEBUG,"pthread_mutex_unlock failed with %d", rc);
        thread_func_args -> thread_complete_success = false;
        return thread_param;
    }
        
    thread_func_args -> thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    struct thread_data *params = (struct thread_data *)malloc(sizeof(struct thread_data));

    memset(params, 0, sizeof(struct thread_data));

    params->wait_to_obtain_ms = wait_to_obtain_ms;
    params->wait_to_release_ms = wait_to_release_ms;
    params->mutex = mutex;

    int rc = pthread_create(thread, NULL, threadfunc, params);
    if(rc != 0){
        syslog(LOG_DEBUG,"pthread_create failed with %d", rc);
        return false;
    }

    return true;
}

