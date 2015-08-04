/* 
 * File:   database.h
 * Author: mk
 *
 * Created on July 31, 2015, 12:08 PM
 */

#ifndef DATABASE_H
#define	DATABASE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include<pthread.h>
typedef struct{
    int db_size;
    int alg_type;
    pthread_barrier_t *ckp_db_barrier;

} db_thread_info;   
 
void *database_thread(void *arg);
int naive_read( int index);
int naive_write( int index,int value);
int ckp_naive( int ckp_id);
int db_naive_init(int db_size);

#ifdef	__cplusplus
}
#endif

#endif	/* DATABASE_H */

