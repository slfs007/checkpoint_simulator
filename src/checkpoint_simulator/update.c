#include"update.h"
#include"database.h"
#include<fcntl.h>
#include<stdio.h>



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
    char log_name[128];
    
    DB_SIZE = db_size;
    
    switch(alg_type){
        case NAIVE_ALG:
            db_write = naive_write;
            db_read = naive_read;
            snprintf(log_name,sizeof(log_name),"./log/naive_update_log_%d",pthread_id);
            break;
        case COPY_ON_UPDATE_ALG:
            db_write = cou_write;
            db_read = cou_read;
            snprintf(log_name,sizeof(log_name),"./log/cou_update_log_%d",pthread_id);
            break;
        default:
            perror("alg_type error");
            break;
    }

    pthread_barrier_wait( update_brr_init);
 
    random_update_db( random_buffer,random_buffer_size,log_name);
    pthread_barrier_wait(&brr_exit);
    pthread_exit(NULL);
}
int random_update_db( int *random_buf,int buf_size,char *log_name)
{
    int i;
    int buf;
    struct timespec log_thread_time_start;
    struct timespec log_thread_time_end;
    
    FILE *log_thread;
    
    log_thread = fopen(log_name,"w+");
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
        
     //   fprintf(log_thread,"%ld,%ld\n",log_thread_time_start.tv_sec,log_thread_time_start.tv_nsec);
     //   fprintf(log_thread,"%ld,%ld\n",log_thread_time_end.tv_sec,log_thread_time_end.tv_nsec);
        i++;
   
        pthread_rwlock_unlock(&DB_STATE_rw_lock);
    }
    fclose(log_thread);
    printf("%d\n",i);
    return 0;
}