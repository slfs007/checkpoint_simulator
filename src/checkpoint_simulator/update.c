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
    int intbuf;
    int i;
    int test_fd;
    
    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else
    {
        perror("alg_type error");
    }
    printf("buffer size:%d\n",random_buffer_size);

}
