#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"database.h"
#include"update.h"



int main( int argc, char *argv[])
{
    FILE *rdf;
    int update_thread_num;
    int db_size;
    int alg_type;
    char *rf_name;
    pthread_t *update_thread_array;
    pthread_t db_thread;
    if ( argc != 5)
    {
        perror("usage:./ckp_cimulator [update thread number] [database size] [algorithm type:0-navie 1-copy on update 2-zigzag 3-pingpong] [random file name]");
    }
    update_thread_num = atoi( argv[1]);
    db_size = atoi( argv[2]);
    alg_type = atoi( argv[3]);
    rf_name = argv[4];
    update_thread_array = (pthread_t *)malloc(sizeof(pthread_t) * update_thread_num);
    //start db_thread
    //start update thread array
    //wait for quit
}
