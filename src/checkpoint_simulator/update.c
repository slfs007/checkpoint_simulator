#include"update.h"
#include"database.h"
#include<fcntl.h>
#include<stdio.h>

int random_update_db( int *random_buf,int buf_size);

int (*db_read)( int index);
int (*db_write)( int index,int value);
static int DB_SIZE;
extern int DB_STATE;
extern pthread_rwlock_t DB_STATE_rw_lock;
extern pthread_barrier_t brr_exit;
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
    pthread_barrier_wait(&brr_exit);
    pthread_exit(NULL);
}
int random_update_db( int *random_buf,int buf_size)
{
    int i;
    int buf;
    
    i = 0;
    while(1)
    {
        pthread_rwlock_rdlock(&DB_STATE_rw_lock);
        if ( 1 != DB_STATE ){
            printf("update thread prepare to exit\n");
            
            break;
        }
        
        buf = random_buf[i%buf_size];
        if ( 0 == (buf % 2)){
            db_read(buf);
        }else{
            db_write(buf,buf);
        }
        i++;
        pthread_rwlock_unlock(&DB_STATE_rw_lock);
    }
    
    return 0;
}