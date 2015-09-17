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
#define NAIVE_ALG 0
#define COPY_ON_UPDATE_ALG 1
#define ZIGZAG_ALG  2
#define PINGPONG_ALG 3
#define MK_ALG  4
typedef struct{
    int db_size;
    int alg_type;
    int unit_size;
    pthread_barrier_t *ckp_db_barrier;
    pthread_barrier_t *brr_db_exit;
} db_thread_info;   
typedef struct{
    int db_size;
    char *db_naive_AS;
    char *db_naive_AS_shandow;
    pthread_rwlock_t write_mutex;

}db_naive_infomation;

typedef struct{
    int db_size;
    char *db_cou_primary;
    char *db_cou_shandow;
    unsigned char *db_cou_bitarray;
    pthread_rwlock_t db_mutex;

}db_cou_infomation;
/*basic*/
void log_time_write( struct timespec *log,int log_size,char *log_name);
void *database_thread(void *arg);

/*naive*/
int db_naive_init(void *db_naive_info,int db_size);
void* naive_read( int index);
int naive_write( int index,void *value);
void ckp_naive( int ckp_id, void *db_naive_info);
void db_naive_destroy( void *db_naive_info);
/*copy on update*/
int db_cou_init(void *cou_info,int db_size);
void *cou_read( int index);
int cou_write( int index, void *value);
void ckp_cou( int ckp_id,void *cou_info);
void db_cou_destroy( void *cou_info);
/*zigzag*/
typedef struct{
    int db_size;
    char *db_zigzag_as0;
    char *db_zigzag_as1;
    unsigned char *db_zigzag_mr;
    unsigned char *db_zigzag_mw;
    pthread_rwlock_t write_mutex;

}db_zigzag_infomation;

int db_zigzag_init(void *cou_info,int db_size);
void* zigzag_read( int index);
int zigzag_write( int index, void *value);
void db_zigzag_ckp( int ckp_order,void *cou_info);
void db_zigzag_destroy( void *cou_info);
/*pingpong*/
typedef struct{
    int db_size;
    char *db_pp_as;
    char *db_pp_as_odd;
    char *db_pp_as_even;
    char *db_pp_as_previous;
    unsigned char *db_pp_odd_ba;
    unsigned char *db_pp_even_ba;
    pthread_rwlock_t write_mutex;
    int current;
}db_pingpong_infomation;

int db_pingpong_init(void *pp_info,int db_size);
void* pingpong_read( int index);
int pingpong_write( int index, void *value);
void db_pingpong_ckp( int ckp_order,void *pp_info);
void db_pingpong_destroy( void *pp_info);

typedef struct{
    int db_size;
    char *db_mk_as1;
    char *db_mk_as2;
    unsigned char *db_mk_ba;
    pthread_rwlock_t db_rwlock;
    int current;
    int lock;
}db_mk_infomation;
int db_mk_init(void *mk_info,int db_size);
void *mk_read( int index);
int mk_write( int index,void* value);
void db_mk_ckp( int ckp_order,void *mk_info);
void db_mk_destroy( void *mk_info);
#ifdef	__cplusplus
}
#endif

#endif	/* DATABASE_H */

