#include"update.h"
#include"database.h"
void *update_thread(void *arg)
{
    int db_size = (( int *)arg)[0];
    int alg_type = (( int *)arg)[1];
    
    if ( 0 == alg_type )
    {
        db_write = naive_write;
        db_read = naive_read;
    }else
    {
        perror("alg_type error");
    }


}