/*
 * File:   database.h
 * Author: mk
 *
 * Created on July 31, 2015, 12:08 PM
 */

#ifndef DATABASE_H
#define	DATABASE_H
#include"include.h"
#define NAIVE_ALG           0
#define COPY_ON_UPDATE_ALG  1
#define ZIGZAG_ALG          2
#define PINGPONG_ALG        3
#define MK_ALG              4
#define LL_ALG              5
typedef struct {
    int dbSize;
    int algType;
  
    pthread_barrier_t *ckpInitBrr;
    pthread_barrier_t *ckpExitBrr;
} db_thread_info;

void log_time_write(db_server *s);
void *database_thread(void *arg);
long long get_ntime(void);
long long get_utime(void);
long long get_mtime(void);

#endif	/* DATABASE_H */

