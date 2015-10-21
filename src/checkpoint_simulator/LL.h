/* 
 * File:   LL.h
 * Author: mk
 *
 * Created on October 21, 2015, 9:31 AM
 */

#ifndef LL_H
#define	LL_H

#include<pthread.h>
typedef struct {
    int db_size;
    char *db_ll_as0;
    char *db_ll_as1;
    char *db_ll_prev;
    unsigned char *db_ll_as0_ba;
    unsigned char *db_ll_as1_ba;
    unsigned char *db_ll_mr_ba;
    pthread_rwlock_t db_rwlock;
    int current;

} db_ll_infomation;
#include"include.h"

int db_ll_init(void *ll_info, int db_size);
void *ll_read(int index);
int ll_write(int index, void *value);
void db_ll_ckp(int ckp_id, void *ll_info);
void db_ll_destroy(void *ll_info);
#endif	/* LL_H */

