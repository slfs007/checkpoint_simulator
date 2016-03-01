/* 
 * File:   pingpong.h
 * Author: mk
 *
 * Created on September 20, 2015, 8:34 PM
 */

#ifndef PINGPONG_H
#define	PINGPONG_H

#include<pthread.h>
typedef struct {
    int db_size;
    char *db_pp_as;
    char *db_pp_as_odd;
    char *db_pp_as_even;
    char *db_pp_as_previous;
    unsigned char *db_pp_odd_ba;
    unsigned char *db_pp_even_ba;
    unsigned char db_pp_lock;
    int current;
} db_pingpong_infomation;
#include"include.h"
int db_pingpong_init(void *pp_info, int db_size);
void* pingpong_read(int index);
int pingpong_write(int index, void *value);
void db_pingpong_ckp(int ckp_order, void *pp_info);
void db_pingpong_destroy(void *pp_info);
#endif	/* PINGPONG_H */

