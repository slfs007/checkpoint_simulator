#include"database.h"
#include<stdio.h>
void *database_thread(void *arg)
{
    int db_size = (( int *)arg)[0];
    int alg_type = (( int *)arg)[1];
    
    printf("database thread start，db_size:%d alg_type:%d\n",db_size,alg_type);
}
int navie_read( int index)
{
}
int navie_write( int index)
{
}

