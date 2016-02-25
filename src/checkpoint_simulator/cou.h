/* 
 * File:   cou.h
 * Author: mk
 *
 * Created on September 20, 2015, 8:24 PM
 */

#ifndef COU_H
#define	COU_H
#include<pthread.h>
typedef struct {
    int db_size;
    char *db_cou_primary;
    char *db_cou_shandow;
    unsigned char *db_cou_curBA;
    unsigned char *db_cou_preBA;
    unsigned char *db_cou_chgBA;
    pthread_rwlock_t db_mutex;

} db_cou_infomation;
#include"include.h"

int db_cou_init(void *cou_info, int db_size);
void *cou_read(int index);
int cou_write(int index, void *value);
void ckp_cou(int ckp_id, void *cou_info);
void db_cou_destroy(void *cou_info);
#endif	/* COU_H */

