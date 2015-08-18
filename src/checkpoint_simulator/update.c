#include"update.h"
#include"database.h"
#include<fcntl.h>
#include<stdio.h>

int random_update_db( int *random_buf,int buf_size,int thread_id);

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
    int pthread_id = (( update_thread_info *)arg)->pthread_id;
    DB_SIZE = db_size;
    
    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else if ( 1 == alg_type){
        db_write = cou_write;
        db_read = cou_read;
    }else{
        perror("alg_type error");
    }

    pthread_barrier_wait( update_brr_init);
 
    random_update_db( random_buffer,random_buffer_size,pthread_id);
    pthread_barrier_wait(&brr_exit);
    pthread_exit(NULL);
}
int random_update_db( int *random_buf,int buf_size,const int thread_id)
{
    int i;
    int buf;
    struct timespec log_thread_time_start;
    struct timespec log_thread_time_end;
    
    FILE *log_thread;
    char str[64];
    
    sprintf(str,"./log/log_update_%d",thread_id);
    log_thread = fopen(str,"w");
    i = 0;
    while(1)
    {
        
        pthread_rwlock_rdlock(&DB_STATE_rw_lock);
        if ( 1 != DB_STATE ){
            printf("update thread prepare to exit\n");
            break;
        }
        buf = random_buf[i%buf_size];
        
        clock_gettime(CLOCK_MONOTONIC, &log_thread_time_start);
        db_write(buf%DB_SIZE,buf);
        clock_gettime(CLOCK_MONOTONIC, &log_thread_time_end);
        
        fprintf(log_thread,"%ld,%ld\n",log_thread_time_start.tv_sec,log_thread_time_start.tv_nsec);
        fprintf(log_thread,"%ld,%ld\n",log_thread_time_end.tv_sec,log_thread_time_end.tv_nsec);
        i++;
   
        pthread_rwlock_unlock(&DB_STATE_rw_lock);
    }
    fclose(log_thread);
    printf("%d\n",i);
    return 0;
}