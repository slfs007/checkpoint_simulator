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

typedef struct {
        int db_size;
        int alg_type;
        int unit_size;
        pthread_barrier_t *ckp_db_barrier;
        pthread_barrier_t *brr_db_exit;
} db_thread_info;

void log_time_write(struct timespec *log, int log_size, char *log_name);
void *database_thread(void *arg);
long long get_ntime(void);
long long get_utime(void);
long long get_mtime(void);

#endif	/* DATABASE_H */

