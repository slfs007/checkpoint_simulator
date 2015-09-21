#include"include.h"
#include"naive.h"
extern db_server DBServer;
int db_naive_init(void *naive_info,int db_size)
{
    db_naive_infomation *info;
    
    info = naive_info;
    info->db_size = db_size;
    if ( NULL == (info->db_naive_AS = 
            ( char *)malloc( DBServer.unitSize * db_size))){
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    
    memset(info->db_naive_AS,'S',DBServer.unitSize * db_size);
    
    if ( NULL == (info->db_naive_AS_shandow = 
            ( char *)malloc( DBServer.unitSize * db_size))){
        perror("db_navie_AS_shandow malloc error");
        return -1;
    }
   
    if ( 0!= pthread_rwlock_init(&(info->write_mutex),NULL))
    {
        perror("write_mutex init error");
        return -1;
    }
    return 0;
}
void db_naive_destroy( void *naive_info)
{
    db_naive_infomation *info;
    info = naive_info;

    pthread_rwlock_destroy(& (info->write_mutex));
    free( info->db_naive_AS);
    free( info->db_naive_AS_shandow);
}
void* naive_read( int index)
{
    void *result;
    if ( index >= DBServer.dbSize){
        index = index % DBServer.dbSize;
    }
    result = ( void *) ((DBServer.naiveInfo).db_naive_AS + index * DBServer.unitSize);
    return result;
}
int naive_write( int index,void *value)
{
    if ( index >= DBServer.dbSize){
        index = index % DBServer.dbSize;
    }
    pthread_rwlock_rdlock(&((DBServer.naiveInfo).write_mutex));
    memcpy((DBServer.naiveInfo).db_naive_AS + index * DBServer.unitSize + index%DBServer.unitSize,value,4);
    pthread_rwlock_unlock(&((DBServer.naiveInfo).write_mutex));
    return 0;
}
void ckp_naive( int ckp_order, void *naive_info)
{
    int ckp_fd;
    char ckp_name[32];
    db_naive_infomation *info;
    int i;
    int db_size;
    
    info = naive_info;
    sprintf(ckp_name,"./ckp_backup/naive_%d",ckp_order);
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;

    pthread_rwlock_wrlock(& (info->write_mutex));
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2]));
    for ( i = 0; i < db_size; i ++){
        memcpy(info->db_naive_AS_shandow + i * DBServer.unitSize,
                info->db_naive_AS + i * DBServer.unitSize, DBServer.unitSize);
    }
   
    pthread_rwlock_unlock(&(info->write_mutex));
    
    write(ckp_fd,info->db_naive_AS_shandow,DBServer.unitSize * db_size);
    fsync(ckp_fd);
    close(ckp_fd);
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2 + 1]));
}
