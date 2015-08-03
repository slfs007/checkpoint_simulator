#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"database.h"
#include"update.h"
#include <sys/mman.h>
#include<fcntl.h>

int db_thread_start(pthread_t *db_thread_id,int alg_type,int db_size);
int update_thread_start(pthread_t * update_thread_id_array,int type_alg,
                        int db_size,int thread_num,char *rf_name);
int main( int argc, char *argv[])
{
    int i;
    int update_thread_num;
    int db_size;
    int alg_type;
    char *rf_path;
    pthread_t *update_thread_array;
    pthread_t db_thread_id;
    
    if ( argc != 5){
        perror("usage:./ckp_cimulator [update thread number] [database size] "
                "[algorithm type:0-navie 1-copy on update 2-zigzag 3-pingpong] "
                "[random file name]");
    }
    update_thread_num = atoi( argv[1]);
    db_size = atoi( argv[2]);
    alg_type = atoi( argv[3]);
    rf_path = argv[4];
   
    //start db_thread

    if (0 != db_thread_start(&db_thread_id,alg_type,db_size)){
        exit(1);
    }
   
    if (0 != update_thread_start(update_thread_array,alg_type,db_size,
                                update_thread_num,rf_path)){
        exit(1);
    }
     //wait for quit
    for ( i = 0;i < update_thread_num ; i ++){
        pthread_join( update_thread_array[i],NULL);
        printf("update thread %d exit!\n",i);
    }
    pthread_join( db_thread_id,NULL);
    exit(1);
}
int db_thread_start(pthread_t *db_thread_id,int alg_type,int db_size)
{
    db_thread_info db_info;
    pthread_barrier_t db_brr_init;
    
    pthread_barrier_init( &db_brr_init, NULL,2);
    db_info.alg_type = alg_type;
    db_info.db_size = db_size;
    db_info.ckp_db_barrier = &db_brr_init;
    
    if ( 0 != pthread_create( db_thread_id, NULL, database_thread, &db_info))
    {
        perror("database thread create error!");
        return -1;
    }
    pthread_barrier_wait( &db_brr_init);
    pthread_barrier_destroy( &db_brr_init);
    return 0;

}
int update_thread_start(pthread_t * update_thread_id_array,int alg_type,
                        int db_size,int thread_num,char *rf_name)
{
    update_thread_info update_info;
    int rdf_fd;
    int i;
    
    update_info.alg_type = alg_type;
    update_info.db_size = db_size;
    
    if ( -1 == ( rdf_fd = open(rf_name, O_RDONLY)))
    {
        perror( "random file open error!\n");
        return -1;
    }
    update_info.random_buffer = ( int *)mmap(NULL,1024 * sizeof(int),
                                    PROT_READ,MAP_LOCKED|MAP_SHARED,rdf_fd,0);
    if ( MAP_FAILED == update_info.random_buffer )
    {
        perror("mmap failed!");
        return -2;
    }
    update_info.random_buffer_size = 1024;
    
    if (NULL == (update_thread_id_array 
                    = (pthread_t *)malloc(sizeof(pthread_t) * thread_num)))
    {
        perror("update thread array malloc error");
    }
    for ( i = 0; i < thread_num ; i++)
    {
        if ( 0 != pthread_create( &(update_thread_id_array[i]), NULL,
                                    update_thread,&update_info))
        {
            perror("update thread %d create error",i);
        }else
        {
            printf("update thread %d create success\n",i);
        }
    }
    return 0;
}