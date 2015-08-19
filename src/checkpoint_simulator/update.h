/* 
 * File:   update.h
 * Author: mk
 *
 * Created on July 31, 2015, 12:09 PM
 */

#ifndef UPDATE_H
#define	UPDATE_H

#ifdef	__cplusplus
extern "C" {
#endif
#include<pthread.h>
typedef struct{
    int db_size;
    int alg_type;
    int *random_buffer;
    int random_buffer_size;
    pthread_barrier_t *update_brr_init;
    int pthread_id;

}update_thread_info;
void *update_thread(void *arg);
int random_update_db( int *random_buf,int buf_size,char *log_name);

#ifdef	__cplusplus
}
#endif

#endif	/* UPDATE_H */

