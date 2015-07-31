/* 
 * File:   database.h
 * Author: mk
 *
 * Created on July 31, 2015, 12:08 PM
 */

#ifndef DATABASE_H
#define	DATABASE_H

#ifdef	__cplusplus
extern "C" {
#endif

void *database_thread(void *arg);
int navie_read( int index);
int navie_write( int index);

#ifdef	__cplusplus
}
#endif


#endif	/* DATABASE_H */

