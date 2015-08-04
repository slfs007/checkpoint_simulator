#include"update.h"
#include"database.h"
#include<fcntl.h>
#include<stdio.h>

void *update_thread(void *arg)
{
    int db_size = (( update_thread_info *)arg) ->db_size ;
    int alg_type = (( update_thread_info *)arg) ->alg_type;
    int *random_buffer = (( update_thread_info *)arg) ->random_buffer;
    int random_buffer_size = (( update_thread_info *)arg) ->random_buffer_size;
    pthread_barrier_t *update_brr_init = (( update_thread_info *)arg)->update_brr_init;
    

    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else
    {
        perror("alg_type error");
    }
    pthread_barrier_wait( update_brr_init);
    pthread_exit(NULL);
}
