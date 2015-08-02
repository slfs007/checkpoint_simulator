#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"database.h"
#include"update.h"



int main( int argc, char *argv[])
{
    FILE *rdf;
    int i;
    int update_thread_num;
    int db_size;
    int alg_type;
    int db_thread_arg[2];
    char *rf_name;
    pthread_t *update_thread_array;
    pthread_t db_thread;
    pthread_barrier_t ckp_db_b;   
    db_thread_info db_thread_infomation;
    
    pthread_barrier_init( &ckp_db_b, NULL,2);
    
    if ( argc != 5)
    {
        perror("usage:./ckp_cimulator [update thread number] [database size] [algorithm type:0-navie 1-copy on update 2-zigzag 3-pingpong] [random file name]");
    }
    update_thread_num = atoi( argv[1]);
    db_size = atoi( argv[2]);
    alg_type = atoi( argv[3]);
    rf_name = argv[4];
   
    //start db_thread
    
    db_thread_infomation.alg_type = alg_type;
    db_thread_infomation.db_size = db_size;
    db_thread_infomation.ckp_db_barrier = &ckp_db_b;
    if ( 0 != pthread_create( &db_thread, NULL, database_thread, &db_thread_infomation))
    {
        perror("database thread create error!");
    }
    
    pthread_barrier_wait( &ckp_db_b);
    //start update thread array
    db_thread_arg[0] = db_size;
    db_thread_arg[1] = alg_type;
    if (NULL == (update_thread_array = (pthread_t *)malloc(sizeof(pthread_t) * update_thread_num)))
    {
        perror("update thread array malloc error");
    }
    for ( i = 0; i < update_thread_num ; i++)
    {
        if ( 0 != pthread_create( &(update_thread_array[i]), NULL,update_thread,db_thread_arg))
        {
            printf("update thread %d create error\n",i);
        }else
        {
            printf("update thread %d create success\n",i);
        }
    }
    
     //wait for quit
    for ( i = 0;i < update_thread_num ; i ++)
    {
        pthread_join( update_thread_array[i],NULL);
    }
    pthread_join( db_thread,NULL);
    exit(1);
}
