#include"database.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
db_naive_infomation db_naive_info;
db_cou_infomation db_cou_info;
static int DB_SIZE;
//

int DB_STATE;
pthread_rwlock_t DB_STATE_rw_lock;

int db_cou_init(void *cou_info,int db_size)
{
    db_cou_infomation *info;
    
    info = cou_info;
    info->db_size = db_size;
    
    if ( NULL == (info->db_cou_primary = 
            ( int *)malloc(sizeof(int) * db_size))){
        perror("db_cou_primary malloc error");
        return -1;
    }
    memset(info->db_cou_primary,'S',sizeof( int) * db_size);
    
    if ( NULL == (info->db_cou_shandow = 
            (int *) malloc(sizeof(int) * db_size))){
        perror("db_cou_shandow malloc error");
        return -1;
    }
    memset(info->db_cou_shandow,'S',sizeof( int) * db_size);
    
    if ( NULL == ( info->db_cou_bitarray = 
            (unsigned char *)malloc(db_size))){
        perror("db_cou_bitarray malloc error");
        return -1;
    }
    memset(info->db_cou_bitarray,0, db_size);
    
    pthread_mutex_init(&(info->db_mutex), NULL);
    
    return 0;
}
int cou_read( int index)
{
    int result;
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    result = db_cou_info.db_cou_primary[index];
    return result;
}
int cou_write( int index, int value)
{
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    
    pthread_mutex_lock( &(db_cou_info.db_mutex));
    db_cou_info.db_cou_bitarray[index] = 1;
    db_cou_info.db_cou_primary[index] = value;
    pthread_mutex_unlock( &(db_cou_info.db_mutex));
    return 0;
}
void ckp_cou( int ckp_id,void *cou_info)
{
    FILE *ckp_file;
    char ckp_name[32];
    int i;
    int db_size;
    db_cou_infomation *info;
    
    info = cou_info;
    sprintf(ckp_name,"./ckp_backup/cou_%d",ckp_id);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;
    pthread_mutex_lock(&(info->db_mutex));
    for (i = 0; i < db_size; i ++){
        if ( 1 == info->db_cou_bitarray[i]){
            info->db_cou_bitarray[i] = 0;
            info->db_cou_shandow[i] = 
                    info->db_cou_primary[i];
        }
    }
    pthread_mutex_unlock(&(info->db_mutex));
    
    fwrite(info->db_cou_shandow,sizeof( int),db_size,ckp_file);
    fflush(ckp_file);
    fclose(ckp_file);
}
void db_cou_destroy( void *cou_info)
{
    db_cou_infomation *info;
    
    info = cou_info;
    
    pthread_mutex_destroy( &(info->db_mutex));
    free(info->db_cou_bitarray);
    free(info->db_cou_shandow);
    free(info->db_cou_primary);
}
int db_naive_init(void *naive_info,int db_size)
{
    db_naive_infomation *info;
    
    info = naive_info;
    info->db_size = db_size;
    if ( NULL == (info->db_naive_AS = 
            ( int *)malloc( sizeof(int) * db_size))){
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    
    memset(info->db_naive_AS,'S',sizeof( int) * db_size);
    
    if ( NULL == (info->db_naive_AS_shandow = 
            ( int *)malloc( sizeof(int) * db_size))){
        perror("db_navie_AS_shandow malloc error");
        return -1;
    }
    if ( 0 != pthread_mutex_init(&(info->naive_db_mutex),NULL))
    {
        perror("navie_db_mutex init error");
        return -1;
    }
    if ( 0!= pthread_rwlock_init(&(info->write_mutex),NULL))
    {
        perror("write_mutex init error");
        return -1;
    }
    return 0;
}
void db_naive_destroy( void *naive_info)
{
    db_naive_infomation *info;
    info = naive_info;
    pthread_mutex_destroy(& (info->naive_db_mutex));
    pthread_rwlock_destroy(& (info->write_mutex));
    free( info->db_naive_AS);
    free( info->db_naive_AS_shandow);
}
void *database_thread(void *arg)
{
    int db_size = ((db_thread_info *)arg)->db_size;
    int alg_type = ((db_thread_info *)arg)->alg_type;
    pthread_barrier_t *brr_exit = ((db_thread_info *)arg)->brr_db_exit;
    int ckp_id;
    int ckp_num;
    
    pthread_barrier_t *ckp_db_b;
    struct timespec ckp_time_log[2000];
    char db_log_name[128];
    int (*db_init)(void *,int);
    void (*checkpoint)( int ,void *);
    void (*db_destroy)(void *);
    void *info;
    
    DB_SIZE = db_size;
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d\n",db_size,alg_type);
    switch ( alg_type)
    {
        case NAIVE_ALG:
            db_init = db_naive_init;
            checkpoint = ckp_naive;
            db_destroy = db_naive_destroy;
            info = &db_naive_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/naive_ckp_log");
            break;
        case COPY_ON_UPDATE_ALG:
            db_init = db_cou_init;
            checkpoint = ckp_cou;
            db_destroy = db_cou_destroy;
            info = &db_cou_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/cou_ckp_log");
            break;
        default:
            printf("alg_type error!");
            goto DB_EXIT;
            break;
    }
    
    if ( 0 != db_init(info,db_size)){
        perror("db thread init error!");
        goto DB_EXIT;
    }
        
    pthread_rwlock_init(&DB_STATE_rw_lock,NULL);
    pthread_rwlock_wrlock(&DB_STATE_rw_lock);
    DB_STATE = 1;
    pthread_rwlock_unlock(&DB_STATE_rw_lock);
    
    printf("db thread init success!\n");
    pthread_barrier_wait( ckp_db_b);
    
    ckp_id = 0;
    ckp_num = 50;
    while( 1)
    {
        clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
        checkpoint(ckp_id%10, info);
        clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
        ckp_id ++;

        if (ckp_id >= ckp_num)
        {
            pthread_rwlock_wrlock(&DB_STATE_rw_lock);
            DB_STATE = 0;
            pthread_rwlock_unlock(&DB_STATE_rw_lock);
            break;
        }
    }
    pthread_barrier_wait(brr_exit);
    
DB_EXIT:
    printf("database thread exit\n");
    pthread_rwlock_destroy(&DB_STATE_rw_lock);
    db_destroy(info);
    log_time_write(ckp_time_log,ckp_num * 2,db_log_name);
    pthread_exit(NULL);
}
void ckp_naive( int ckp_id, void *naive_info)
{
    FILE *ckp_file;
    char ckp_name[32];
    db_naive_infomation *info;
    
    info = naive_info;
    sprintf(ckp_name,"./ckp_backup/naive_%d",ckp_id);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }

    pthread_mutex_lock(& (info->naive_db_mutex));
    memcpy(info->db_naive_AS_shandow,info->db_naive_AS,sizeof( int) * DB_SIZE);
    pthread_mutex_unlock(&(info->naive_db_mutex));
    
    fwrite(info->db_naive_AS_shandow,sizeof( int),DB_SIZE,ckp_file);
    fflush(ckp_file);
    fclose(ckp_file);
}
int naive_read( int index)
{
    int result;
    if ( index >= DB_SIZE)
        index = index % DB_SIZE;
    
    pthread_rwlock_rdlock(&(db_naive_info.write_mutex));
    result =  db_naive_info.db_naive_AS[index];
    pthread_rwlock_unlock(&(db_naive_info.write_mutex));
    
    return result;
}
int naive_write( int index,int value)
{
    if ( index >= DB_SIZE)
    {
        index = index % DB_SIZE;
    }
    pthread_mutex_lock(&(db_naive_info.naive_db_mutex));
    pthread_rwlock_wrlock(&(db_naive_info.write_mutex));
    db_naive_info.db_naive_AS[index] = value;
    pthread_rwlock_unlock(&(db_naive_info.write_mutex));
    pthread_mutex_unlock(&(db_naive_info.naive_db_mutex));
    return 0;
}
void log_time_write( struct timespec *log,int log_size,char *log_name)
{
    FILE *log_time;
    int i;
  
   if ( NULL == (log_time = fopen(log_name,"w"))){
        perror("log_time fopen error,checkout if the floder is exist");
        return;
    }
    for (i = 0; i < log_size; i ++)
    {
        fprintf(log_time,"%ld,%ld\n",log[i].tv_sec,log[i].tv_nsec);
    }
    fflush(log_time);
    fclose(log_time);
}