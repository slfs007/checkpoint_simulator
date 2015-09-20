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
//#define COHERENCE_CHECK
#include<pthread.h>
#include"include.h"
typedef struct{
    int db_size;
    int alg_type;
    int *random_buffer;
    int random_buffer_size;
    pthread_barrier_t *update_brr_init;
    pthread_barrier_t *brr_exit;
    int pthread_id;
    int update_frequency;

}update_thread_info;
extern db_server DBServer;
void *update_thread(void *arg);
int random_update_db( int *random_buf,int buf_size,char *log_name,int uf);

#ifdef	__cplusplus
}
#endif

#endif	/* UPDATE_H */

