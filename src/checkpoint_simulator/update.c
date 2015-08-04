#include"update.h"
#include"database.h"
#include<fcntl.h>
#include<stdio.h>

int random_update_db( int *random_buf,int buf_size);

int (*db_read)( int index);
int (*db_write)( int index,int value);
static int DB_SIZE;
void *update_thread(void *arg)
{
    int db_size = (( update_thread_info *)arg) ->db_size ;
    int alg_type = (( update_thread_info *)arg) ->alg_type;
    int *random_buffer = (( update_thread_info *)arg) ->random_buffer;
    int random_buffer_size = (( update_thread_info *)arg) ->random_buffer_size;
    pthread_barrier_t *update_brr_init = (( update_thread_info *)arg)->update_brr_init;
    
    DB_SIZE = db_size;
    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else
    {
        perror("alg_type error");
    }

    pthread_barrier_wait( update_brr_init);
 
    random_update_db( random_buffer,random_buffer_size);
    pthread_exit(NULL);
}
int random_update_db( int *random_buf,int buf_size)
{
    int i;
    int buf;
    for( i = 0; i < buf_size; i++)
    {
        buf = random_buf[i];
        if ( 0 == buf / 2){
            db_read(buf%DB_SIZE);
        }else{
            db_write(buf%DB_SIZE,buf);
        }
    }
    return 0;
}