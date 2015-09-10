#include"update.h"
#include"include.h"


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
    pthread_barrier_t *brr_exit = (( update_thread_info *)arg)->brr_exit;
    int pthread_id = (( update_thread_info *)arg)->pthread_id;
    int update_frequency = (( update_thread_info *)arg)->update_frequency;
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
        case ZIGZAG_ALG:
            db_write = zigzag_write;
            db_read = zigzag_read;
            snprintf(log_name,sizeof(log_name),"./log/zigzag_update_log_%d",pthread_id);
            
            break;
        case PINGPONG_ALG:
            db_write = pingpong_write;
            db_read = pingpong_read;
            snprintf(log_name,sizeof(log_name),"./log/pingpong_update_log_%d",pthread_id);
            
            break;
        default:
            perror("alg_type error");
            break;
    }

    pthread_barrier_wait( update_brr_init);
    random_update_db( random_buffer,random_buffer_size,log_name,update_frequency);
    
    pthread_barrier_wait( brr_exit);
    
    pthread_exit(NULL);
}
int execute_update(int *random_buf,int buf_size,int times,FILE *log)
{
    int i;
    int buf;
    struct timespec time_start;
    struct timespec time_end;
    
    for (i = 0; i < times; i ++){
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        pthread_rwlock_rdlock(&DB_STATE_rw_lock);
        if ( 1 != DB_STATE ){
            printf("update thread prepare to exit\n");
            return -1;
        }
        buf = random_buf[i%buf_size];
//        clock_gettime(CLOCK_MONOTONIC, &log_thread_time_start);
        db_write(buf%DB_SIZE,buf);
//        clock_gettime(CLOCK_MONOTONIC, &log_thread_time_end);
        pthread_rwlock_unlock(&DB_STATE_rw_lock);
        clock_gettime(CLOCK_MONOTONIC, &time_end);
        fprintf(log,"%ld,%ld\n%ld,%ld\n",time_start.tv_sec,time_start.tv_nsec,
                time_end.tv_sec,time_end.tv_nsec);
    }
    return 0;
}
int random_update_db( int *random_buf,int buf_size,char *log_name,int uf)
{
    int i;
    int tick;
    FILE *log_thread;
 
    struct timespec time_now;
    struct timespec time_start;
    long int time_now_us;
    long int time_start_us;
    long int time_tick_us;
    long int time_begin_us;
    long int time_diff;
    log_thread = fopen(log_name,"w+");
   
    tick = 0;
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    time_begin_us = time_start.tv_sec * 1000000 + time_start.tv_nsec / 1000;
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        time_start_us = time_start.tv_sec * 1000000 + time_start.tv_nsec / 1000;
       
        for (i = 0; i < 1000; i ++){
            if (-1 == execute_update(random_buf,buf_size,uf/1000,log_thread)){
                clock_gettime(CLOCK_MONOTONIC, &time_now);
                goto EXIT;
            }
            clock_gettime(CLOCK_MONOTONIC, &time_now);
            time_now_us = time_now.tv_sec * 1000000 + time_now.tv_nsec / 1000;
            time_tick_us = time_start_us + i * 1000;
            if ( time_now_us < time_tick_us){
                usleep(time_tick_us - time_now_us);
            }
        }
        tick ++;
    }
   //clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2])); 
EXIT:
    fclose(log_thread);
    tick = tick * uf + i * (uf/1000);
    time_now_us = time_now.tv_sec * 1000000 + time_now.tv_nsec / 1000;
    time_diff = (time_now_us - time_begin_us)/1000000;
    
    printf("set uf:%d,real uf:%ld\n",uf,time_diff == 0 ? 0: tick/time_diff);
    return 0;
}