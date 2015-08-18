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
    pthread_rwlock_t write_mutex;

}db_naive_infomation;

typedef struct{
    int db_size;
    int *db_cou_primary;
    int *db_cou_shandow;
    unsigned char *db_cou_bitarray;
    pthread_rwlock_t *db_cou_lock;
    pthread_mutex_t db_mutex;
    
}db_cou_infomation;
/*basic*/
void log_time_write( struct timespec *log,int log_size);
void *database_thread(void *arg);

/*naive*/
int db_naive_init(db_naive_infomation *db_naive_info,int db_size);
int naive_read( int index);
int naive_write( int index,int value);
void inline ckp_naive( int ckp_id, db_naive_infomation *db_naive_info);
void db_naive_destroy( db_naive_infomation *db_naive_info);
/*copy on update*/
int db_cou_init(db_cou_infomation *db_cou_info,int db_size);
int cou_read( int index);
int cou_write( int index, int value);
void inline ckp_cou( int ckp_id,db_cou_infomation *db_cou_info);
void db_cou_destroy( db_cou_infomation *db_naive_info);

#ifdef	__cplusplus
}
#endif

#endif	/* DATABASE_H */

