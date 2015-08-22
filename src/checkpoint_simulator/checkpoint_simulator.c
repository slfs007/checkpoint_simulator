#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"database.h"
#include"update.h"
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>

int db_thread_start(pthread_t *db_thread_id,int alg_type,int db_size,
        pthread_barrier_t *brr_exit);
int update_thread_start(pthread_t *update_thread_id_array[],const int alg_type,
                        const int db_size,const int thread_num,
                        int *map_buf,const int map_size,
                        pthread_barrier_t *brr_exit);
int main( int argc, char *argv[])
{
    int i;
    int update_thread_num;
    int db_size;
    int alg_type;
    int *map_buf;
    char *rf_path;
    int rdf_fd;
    pthread_t *update_thread_array;
    pthread_t db_thread_id;
    pthread_barrier_t brr_exit;
    
    if ( argc != 5){
        perror("usage:./ckp_cimulator [update thread number] [database size] "
                "[algorithm type:0-navie 1-copy on update 2-zigzag 3-pingpong] "
                "[random file name]");
    }
    
    update_thread_num = atoi( argv[1]);
    db_size = atoi( argv[2]);
    alg_type = atoi( argv[3]);
    rf_path = argv[4];
   
    //init the brr_exit
    pthread_barrier_init(&brr_exit,NULL, update_thread_num + 1);
    //start db_thread

    if (0 != db_thread_start(&db_thread_id,alg_type,db_size,&brr_exit)){
        perror("db thread start fail!");
        exit(1);
    }
   
    if ( -1 == ( rdf_fd = open(rf_path, O_RDONLY)))
    {
        perror( "random file open error!\n");
        return -1;
    }
   
    map_buf = ( int *)mmap(NULL,1024 * sizeof(int),
                                    PROT_READ,MAP_LOCKED|MAP_SHARED,rdf_fd,0);

    if ( MAP_FAILED == map_buf )
    {
        perror("mmap failed!");
        return -2;
    }
    if (0 != update_thread_start(&update_thread_array,alg_type,db_size,
                                update_thread_num,map_buf,1024,&brr_exit)){
        return -3;
    }
     //wait for quit
    pthread_join( db_thread_id,NULL);
    for ( i = 0;i < update_thread_num ; i ++){
        
        pthread_join(update_thread_array[i],NULL);
        printf("update thread %d exit!\n",i);
    }
    free(update_thread_array);
    close(rdf_fd);
    pthread_barrier_destroy(&brr_exit);
    exit(1);
}
int db_thread_start(pthread_t *db_thread_id,int alg_type,int db_size,pthread_barrier_t *brr_exit)
{
    db_thread_info db_info;
    pthread_barrier_t db_brr_init;
    
    pthread_barrier_init( &db_brr_init, NULL,2);
    db_info.alg_type = alg_type;
    db_info.db_size = db_size;
    db_info.ckp_db_barrier = &db_brr_init;
    db_info.brr_db_exit = brr_exit;
    if ( 0 != pthread_create( db_thread_id, NULL, database_thread, &db_info))
    {
        perror("database thread create error!");
        return -1;
    }

    pthread_barrier_wait( &db_brr_init);
    pthread_barrier_destroy( &db_brr_init);

    return 0;
}
int update_thread_start(pthread_t *update_thread_id_array[],const int alg_type,
                        const int db_size,const int thread_num,
                        int *map_buf,const int map_size,
                        pthread_barrier_t *brr_exit)
{

    int i;
    update_thread_info update_info;
    pthread_barrier_t update_brr_init;
    
    update_info.alg_type = alg_type;
    update_info.db_size = db_size;
    update_info.random_buffer = map_buf;
    update_info.random_buffer_size = map_size;
    pthread_barrier_init( &update_brr_init, NULL,thread_num  +1);
    update_info.update_brr_init = &update_brr_init;
    update_info.brr_exit = brr_exit;
    
    printf("thread num:%d\n",thread_num);
    if (NULL == ((*update_thread_id_array) 
                    = (pthread_t *)malloc(sizeof(pthread_t) * thread_num)))
    {
        perror("update thread array malloc error");
    }
    for ( i = 0; i < thread_num ; i++)
    {
        update_info.pthread_id = i;
        if ( 0 != pthread_create( &((*update_thread_id_array)[i]), 
                NULL,update_thread,&update_info))
        {
            printf("update thread %d create error",i);
        }else
        {
            printf("update thread %d create success\n",i);
        }
    }
    pthread_barrier_wait( &update_brr_init);
    pthread_barrier_destroy( &update_brr_init);
    return 0;

}