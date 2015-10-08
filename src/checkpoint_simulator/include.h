/* 
 * File:   include.h
 * Author: mk
 *
 * Created on August 22, 2015, 6:08 PM
 */

#ifndef INCLUDE_H
#define	INCLUDE_H


#include<fcntl.h>
#include<stdio.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>    
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#include"naive.h"
#include"cou.h"
#include"zigzag.h"
#include"pingpong.h"
#include"mk.h"

typedef struct {
    int algType;
    int dbSize;
    int unitSize;
    struct timespec ckpTimeLog[2000];
    long long *ckpOverheadLog;
    long long *ckpPrepareLog;
    long long *ckpTotalOverheadLog;
    int ckpMaxNum;
    int ckpID;
    int dbState;
    int updateThrNum;
    int updateFrequency;
    int *rfBuf;
    int rfBufSize;
    pthread_rwlock_t dbStateRWLock;
    db_naive_infomation naiveInfo;
    db_cou_infomation couInfo;
    db_zigzag_infomation zigzagInfo;
    db_pingpong_infomation pingpongInfo;
    db_mk_infomation mkInfo;

} db_server;

#include"database.h"
#include"update.h"

void add_overhead_log(db_server *s,long long ns);
void add_prepare_log(db_server *s,long long ns);
void add_total_log(db_server *s,long long ns);
int db_thread_start(pthread_t *db_thread_id, pthread_barrier_t *brr_exit, db_server *dbs);
int update_thread_start(pthread_t *update_thread_id_array[],
	pthread_barrier_t *brr_exit,
	db_server *dbs);
void write_overhead_log(db_server *s,const char *filePath);
int randomfile_init(FILE *rf,int *rbuf,int rbufSize);
#endif	/* INCLUDE_H */

