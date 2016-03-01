/* 
 * File:   zigzag.h
 * Author: mk
 *
 * Created on September 20, 2015, 8:02 PM
 */

#ifndef ZIGZAG_H
#define	ZIGZAG_H
#include<pthread.h>
typedef struct {
    int db_size;
    char *db_zigzag_as0;
    char *db_zigzag_as1;
    unsigned char *db_zigzag_mr;
    unsigned char *db_zigzag_mw;
    unsigned char db_zigzag_lock;

} db_zigzag_infomation;
#include"include.h"
int db_zigzag_init(void *cou_info, int db_size);
void* zigzag_read(int index);
int zigzag_write(int index, void *value);
void db_zigzag_ckp(int ckp_order, void *cou_info);
void db_zigzag_destroy(void *cou_info);
#endif	/* ZIGZAG_H */

