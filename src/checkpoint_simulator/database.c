#include"database.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
db_naive_infomation db_naive_info;
db_cou_infomation db_cou_info;
static int DB_SIZE;

int DB_STATE;
pthread_rwlock_t DB_STATE_rw_lock;

extern pthread_barrier_t brr_exit;

int db_cou_init(db_cou_infomation *db_cou_info,int db_size)
{
    int i;
    db_cou_info->db_size = db_size;
    
    if ( NULL == (db_cou_info->db_cou_primary = 
            ( int *)malloc(sizeof(int) * db_size))){
        perror("db_cou_primary malloc error");
        return -1;
    }
    memset(db_cou_info->db_cou_primary,'S',sizeof( int) * db_size);
    
    if ( NULL == (db_cou_info->db_cou_shandow = 
            (int *) malloc(sizeof(int) * db_size))){
        perror("db_cou_shandow malloc error");
        return -1;
    }
    memset(db_cou_info->db_cou_shandow,'S',sizeof( int) * db_size);
    
    if ( NULL == ( db_cou_info->db_cou_bitarray = 
            (unsigned char *)malloc(db_size))){
        perror("db_cou_bitarray malloc error");
        return -1;
    }
    memset(db_cou_info->db_cou_bitarray,0, db_size);
    
    if ( NULL == (db_cou_info->db_cou_lock = 
            (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t) * db_size))){
        perror("db_cou_lock malloc error" );
        return -1;
    }
    
    for (i = 0; i < db_size; i++){
        pthread_rwlock_init( &(db_cou_info->db_cou_lock[i]),NULL);
    }
    pthread_mutex_init(&(db_cou_info->db_mutex), NULL);
    
    return 0;
}
int cou_read( int index)
{
    int result;
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    pthread_rwlock_rdlock( &(db_cou_info.db_cou_lock[index]));
    result = db_cou_info.db_cou_primary[index];
    pthread_rwlock_unlock( &(db_cou_info.db_cou_lock[index]));
    return result;
}
int cou_write( int index, int value)
{
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    
    pthread_mutex_lock( &(db_cou_info.db_mutex));
    pthread_rwlock_wrlock( &(db_cou_info.db_cou_lock[index]));
    db_cou_info.db_cou_bitarray[index] = 1;
    db_cou_info.db_cou_primary[index] = value;
    pthread_rwlock_unlock( &(db_cou_info.db_cou_lock[index]));
    pthread_mutex_unlock( &(db_cou_info.db_mutex));
    return 0;
}
void inline ckp_cou( int ckp_id,db_cou_infomation *db_cou_info)
{
    FILE *ckp_file;
    char ckp_name[32];
    int i;
    int db_size;
    sprintf(ckp_name,"./ckp_backup/%d",ckp_id);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = db_cou_info->db_size;
    pthread_mutex_lock(&(db_cou_info->db_mutex));
    for (i = 0; i < db_size; i ++){
        if ( 1 == db_cou_info->db_cou_bitarray[i]){
            db_cou_info->db_cou_bitarray[i] = 0;
            db_cou_info->db_cou_shandow[i] = 
                    db_cou_info->db_cou_primary[i];
        }
    }
    pthread_mutex_unlock(&(db_cou_info->db_mutex));
    
    fwrite(db_cou_info->db_cou_shandow,sizeof( int),db_size,ckp_file);
    fflush(ckp_file);
    fclose(ckp_file);
}
void db_cou_destroy( db_cou_infomation *db_cou_info)
{
    int i;
    
    for (i = 0; i < db_cou_info->db_size ; i++){
        pthread_rwlock_destroy(&(db_cou_info->db_cou_lock[i]));
    }
    
    pthread_mutex_destroy( &(db_cou_info->db_mutex));
    free(db_cou_info->db_cou_bitarray);
    free(db_cou_info->db_cou_lock);
    free(db_cou_info->db_cou_primary);
    free(db_cou_info->db_cou_shandow);

}
int db_naive_init(db_naive_infomation *db_naive_info,int db_size)
{
    
    db_naive_info->db_size = db_size;
    if ( NULL == (db_naive_info->db_naive_AS = 
            ( int *)malloc( sizeof(int) * db_size))){
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    
    memset(db_naive_info->db_naive_AS,'S',sizeof( int) * db_size);
    
    if ( NULL == (db_naive_info->db_naive_AS_shandow = 
            ( int *)malloc( sizeof(int) * db_size))){
        perror("db_navie_AS_shandow malloc error");
        return -1;
    }
    if ( 0 != pthread_mutex_init(&(db_naive_info->naive_db_mutex),NULL))
    {
        perror("navie_db_mutex init error");
        return -1;
    }
    if ( 0!= pthread_rwlock_init(&(db_naive_info->write_mutex),NULL))
    {
        perror("write_mutex init error");
        return -1;
    }
    return 0;
}
void db_naive_destroy( db_naive_infomation *db_naive_info)
{
    pthread_mutex_destroy(& (db_naive_info->naive_db_mutex));
    pthread_rwlock_destroy(& (db_naive_info->write_mutex));
    free( db_naive_info->db_naive_AS);
    free( db_naive_info->db_naive_AS_shandow);
}
void *database_thread(void *arg)
{
    int db_size = ((db_thread_info *)arg)->db_size;
    int alg_type = ((db_thread_info *)arg)->alg_type;
    int ckp_id;
    int ckp_num;
    pthread_barrier_t *ckp_db_b;
    struct timespec ckp_time_log[2000];
    
    
    DB_SIZE = db_size;
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d\n",db_size,alg_type);
    if ( 0 == alg_type ){
        if (0 != db_naive_init(&db_naive_info,db_size)){
            perror("db_navie_init error");
            goto DB_EXIT;
        }
    } else if(1 == alg_type){ 
        if (0 != db_cou_init(&db_cou_info,db_size)){
            perror("db_cou_init error");
            goto DB_EXIT;
        }
    }else{
        perror("alg type error");
        goto DB_EXIT;
    }
    
    pthread_rwlock_init(&DB_STATE_rw_lock,NULL);
    pthread_rwlock_wrlock(&DB_STATE_rw_lock);
    DB_STATE = 1;
    pthread_rwlock_unlock(&DB_STATE_rw_lock);
    
    printf("database thread init success!\n");
    pthread_barrier_wait( ckp_db_b);
    
    ckp_id = 0;
    ckp_num = 500;
    while( 1)
    {
        clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
        switch(alg_type)
        {
            case 0:
                ckp_naive(ckp_id%10,&db_naive_info);
                break;
            case 1:
                ckp_cou(ckp_id % 10, &db_cou_info);
                break;
                
        }
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

DB_EXIT:
    //barrier
    pthread_barrier_wait(&brr_exit);
    printf("database thread exit\n");
    pthread_rwlock_destroy(&DB_STATE_rw_lock);
    switch ( alg_type)
    {
        case 0:
            db_naive_destroy( &db_naive_info);
            break;
        case 1:
            db_cou_destroy( &db_cou_info);
            break;
    }
    log_time_write(ckp_time_log,ckp_num * 2);
    pthread_exit(NULL);
}
void inline ckp_naive( int ckp_id, db_naive_infomation *db_naive_info)
{
    FILE *ckp_file;
    char ckp_name[32];
    
    
    sprintf(ckp_name,"./ckp_backup/%d",ckp_id);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }

    pthread_mutex_lock(& (db_naive_info->naive_db_mutex));
    memcpy(db_naive_info->db_naive_AS_shandow,db_naive_info->db_naive_AS,sizeof( int) * DB_SIZE);
    pthread_mutex_unlock(&(db_naive_info->naive_db_mutex));
    
    fwrite(db_naive_info->db_naive_AS_shandow,sizeof( int),DB_SIZE,ckp_file);
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
void log_time_write( struct timespec *log,int log_size)
{
    FILE *log_time;
    int i;

   if ( NULL == (log_time = fopen("./log/db_log_time","w"))){
        perror("log_time fopen error,checkout if the floder is exist");
        return;
    }
    for (i = 0; i < log_size; i ++)
    {
        fprintf(log_time,"%ld,%ld\n",log[i].tv_sec,log[i].tv_nsec);
    }
}