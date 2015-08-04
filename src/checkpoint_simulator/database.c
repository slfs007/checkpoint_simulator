#include"database.h"
#include<stdio.h>

#include<stdlib.h>
#include<string.h>
static int *db_naive_AS;
static int *db_naive_AS_shandow;
static int DB_SIZE;
static pthread_mutex_t naive_db_mutex;
static pthread_mutex_t write_mutex;

int db_naive_init(int db_size)
{
    
    if ( NULL == (db_naive_AS = ( int *)malloc( sizeof(int) * db_size)))
    {
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    DB_SIZE = db_size;
    memset(db_naive_AS,'S',sizeof( int) * db_size);
    
    if ( NULL == (db_naive_AS_shandow = ( int *)malloc( sizeof(int) * db_size)))
    {
        perror("db_navie_AS_shandow malloc error");
        return -1;
    }
    if ( 0 != pthread_mutex_init(&naive_db_mutex,NULL))
    {
        perror("navie_db_mutex init error");
        return -1;
    }
    if ( 0!= pthread_mutex_init(&write_mutex,NULL))
    {
        perror("write_mutex init error");
        return -1;
    }
    return 0;
}
void *database_thread(void *arg)
{
    int db_size = ((db_thread_info *)arg)->db_size;
    int alg_type = ((db_thread_info *)arg)->alg_type;
    int ckp_id;
    pthread_barrier_t *ckp_db_b;
    
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d\n",db_size,alg_type);
    if ( 0 == alg_type )
    {
        if (0 != db_naive_init(db_size))
        {
            perror("db_navie_init error");
            goto DB_EXIT;
        }
    }else
    {
        perror("alg type error");
        goto DB_EXIT;
    }
    printf("database thread init success!\n");
    pthread_barrier_wait( ckp_db_b);
    
    ckp_id = 0;
    while( 1)
    {
        ckp_naive(ckp_id%10);
        ckp_id ++;
        if (ckp_id > 1000)
            break;
    }
DB_EXIT:
    printf("database thread exit\n");
    pthread_mutex_destroy(&naive_db_mutex);
    pthread_mutex_destroy(&write_mutex);
    
    free(db_naive_AS);
    free(db_naive_AS_shandow);
    pthread_exit(NULL);
}
int ckp_naive( int ckp_id)
{
    FILE *ckp_file;
    char ckp_name[32];
    
    
    sprintf(ckp_name,"./ckp_backup/%d",ckp_id);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
    }

    
    pthread_mutex_lock(&naive_db_mutex);
    memcpy(db_naive_AS_shandow,db_naive_AS,sizeof( int) * DB_SIZE);
    pthread_mutex_unlock(&naive_db_mutex);
    
    fwrite(db_naive_AS_shandow,sizeof( int),DB_SIZE,ckp_file);
    fflush(ckp_file);
    fclose(ckp_file);
    return 0;
}
int naive_read( int index)
{
    return db_naive_AS[index];
}
int naive_write( int index,int value)
{
    if ( index >= DB_SIZE)
    {
        return -1;
    }
    pthread_mutex_lock(&naive_db_mutex);
    pthread_mutex_lock(&write_mutex);
    
    db_naive_AS[index] = value;
    
    pthread_mutex_unlock(&write_mutex);
    pthread_mutex_unlock(&naive_db_mutex);
    return 0;
}