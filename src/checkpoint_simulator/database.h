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
typedef struct{
    int db_size;
    int *db_naive_AS;
    int *db_naive_AS_shandow;
    pthread_mutex_t naive_db_mutex;
    pthread_mutex_t write_mutex;

}db_naive_infomation;
void *database_thread(void *arg);
int naive_read( int index);
int naive_write( int index,int value);
void inline ckp_naive( int ckp_id, db_naive_infomation *db_naive_info);
void log_time_write( struct timespec *log,int log_size);


#ifdef	__cplusplus
}
#endif

#endif	/* DATABASE_H */

