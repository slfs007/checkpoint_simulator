/* 
 * File:   naive.h
 * Author: mk
 *
 * Created on September 20, 2015, 8:02 PM
 */

#ifndef NAIVE_H
#define	NAIVE_H
#include<pthread.h>
typedef struct {
    int db_size;
    char *db_naive_AS;
    char *db_naive_AS_shandow;
    pthread_rwlock_t write_mutex;
} db_naive_infomation;
#include"include.h"

int db_naive_init(void *db_naive_info, int db_size);
void* naive_read(int index);
int naive_write(int index, void *value);
void ckp_naive(int ckp_id, void *db_naive_info);
void db_naive_destroy(void *db_naive_info);
#endif	/* NAIVE_H */

