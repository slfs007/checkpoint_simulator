#include"cou.h"
extern db_server DBServer;
int db_cou_init(void *cou_info,int db_size)
{
    db_cou_infomation *info;
    
    info = cou_info;
    info->db_size = db_size;
    
    if ( NULL == (info->db_cou_primary = 
            ( char *)malloc(DBServer.unitSize * db_size))){
        perror("db_cou_primary malloc error");
        return -1;
    }
    memset(info->db_cou_primary,'S',DBServer.unitSize * db_size);
    
    if ( NULL == (info->db_cou_shandow = 
            (char *) malloc(DBServer.unitSize * db_size))){
        perror("db_cou_shandow malloc error");
        return -1;
    }
    memset(info->db_cou_shandow,'S',DBServer.unitSize * db_size);
    
    if ( NULL == ( info->db_cou_bitarray = 
            (unsigned char *)malloc(db_size))){
        perror("db_cou_bitarray malloc error");
        return -1;
    }
    memset(info->db_cou_bitarray,0, db_size);
    
    pthread_rwlock_init(&(info->db_mutex), NULL);
    
    return 0;
}
void* cou_read( int index)
{
    void *result;
    if ( index > DBServer.dbSize)
        index = index % DBServer.dbSize;
    result = (DBServer.couInfo).db_cou_primary + index * DBServer.unitSize;
    return result;
}
int cou_write( int index, void *value)
{
    if ( index > DBServer.dbSize)
        index = index % DBServer.dbSize;
    
    pthread_rwlock_rdlock( &((DBServer.couInfo).db_mutex));
    
    (DBServer.couInfo).db_cou_bitarray[index] = 1;
    memcpy((DBServer.couInfo).db_cou_primary + index * DBServer.unitSize + index % DBServer.unitSize,value,4);
    pthread_rwlock_unlock( &((DBServer.couInfo).db_mutex));
    return 0;
}
void ckp_cou( int ckp_order,void *cou_info)
{
    int ckp_fd;
    char ckp_name[32];
    int i;
    int db_size;
    db_cou_infomation *info;
    
    info = cou_info;
    sprintf(ckp_name,"./ckp_backup/cou_%d",ckp_order);
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;
    pthread_rwlock_wrlock(&(info->db_mutex));
    
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2]));
    for (i = 0; i < db_size; i ++){
        if ( 1 == info->db_cou_bitarray[i]){
            info->db_cou_bitarray[i] = 0;
            memcpy(info->db_cou_shandow + i * DBServer.unitSize,
                    info->db_cou_primary + i * DBServer.unitSize,DBServer.unitSize);

        }
    }
    pthread_rwlock_unlock(&(info->db_mutex));
    
    write(ckp_fd,info->db_cou_shandow, DBServer.unitSize*db_size);
    fsync(ckp_fd);
    close(ckp_fd);
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2 + 1]));
}
void db_cou_destroy( void *cou_info)
{
    db_cou_infomation *info;
    
    info = cou_info;
    
    pthread_rwlock_destroy( &(info->db_mutex));
    free(info->db_cou_bitarray);
    free(info->db_cou_shandow);
    free(info->db_cou_primary);
}
