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
    int i;
    int buf;
    
    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else
    {
        perror("alg_type error");
    }
    for ( i = 0; i < random_buffer_size; i ++)
    {
        buf = random_buffer[ i];
        if ( 0 == buf/2)
        {
            db_read(buf%db_size);
        }else
        {
            db_write(buf%db_size,buf);
        }
    }

    return;
}
