#include"mk.h"
extern db_server DBServer;
int db_mk_init(void *mk_info,int db_size)
{
    db_mk_infomation *info = mk_info;
    
    info->db_size = db_size;
    
    if ( NULL == (info->db_mk_as1 = malloc( DBServer.unitSize * db_size))){
        perror("db_mk_as1 malloc error");
        return -1;
    }
    memset ( info->db_mk_as1,'S', DBServer.unitSize * db_size);
    
    if ( NULL == (info->db_mk_as2 = malloc(DBServer.unitSize * db_size))){
        perror("db_mk_as2 malloc error");
        return -1;
    }
    memset ( info->db_mk_as2,'S', DBServer.unitSize * db_size);
    
    if ( NULL == (info->db_mk_ba = malloc(db_size))){
        perror("db_mk_ba malloc error");
        return -1;
    }
    memset ( info->db_mk_ba,0, db_size);
    
    pthread_rwlock_init(&(info->db_rwlock),NULL);
    info->current = 1;
    info->lock = 0;
    return 0;
    
}
void* mk_read( int index)
{
    if (index > (DBServer.mkInfo).db_size)
        index = index % (DBServer.mkInfo).db_size;
    if (1 == (DBServer.mkInfo).current){
        return (DBServer.mkInfo).db_mk_as1 + index * DBServer.unitSize;
    }else if(2 == (DBServer.mkInfo).current){
        return (DBServer.mkInfo).db_mk_as2 + index * DBServer.unitSize;
    }else{
        printf("ERROR:MK_READ!\n");
    }
    return NULL;
}
int mk_write( int index, void* value)
{
    if (index > (DBServer.mkInfo).db_size)
        index = index % (DBServer.mkInfo).db_size;
    
    pthread_rwlock_rdlock(&((DBServer.mkInfo).db_rwlock));
  
    if ( 1 == (DBServer.mkInfo).current){
        
      
        memcpy((DBServer.mkInfo).db_mk_as1 + index * DBServer.unitSize + index % DBServer.unitSize,value,4);
        (DBServer.mkInfo).db_mk_ba[index] = 1;
    }else {
        
       
        memcpy((DBServer.mkInfo).db_mk_as2 + index * DBServer.unitSize + index % DBServer.unitSize,value,4);
        (DBServer.mkInfo).db_mk_ba[index] = 2;
    }
    pthread_rwlock_unlock(&((DBServer.mkInfo).db_rwlock));
    return 0;
}
void db_mk_ckp( int ckp_order,void *mk_info)
{
    int ckp_fd;
    char ckp_name[32];
    int i;
    int db_size;
    db_mk_infomation *info;
    
    info = mk_info;
    sprintf(ckp_name,"./ckp_backup/mk_%d",ckp_order);
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT | O_SYNC,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;

    pthread_rwlock_wrlock(&(info->db_rwlock));
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2]));
    //prepare for checkpoint
    info->current = (1 == (info->current)) ? 2 : 1;
    info->lock = 1;
    pthread_rwlock_unlock(&(info->db_rwlock));
    //write to disk
    if ( 1 == info->current)
    {
        for (i = 0; i < db_size; i ++){
            if ( 0 == info->db_mk_ba[i]){
                continue;
            }else if ( 1 == info->db_mk_ba[i]){
                continue;
            }else if ( 2 == info->db_mk_ba[i]){
                //info->db_mk_as1[i] = info->db_mk_as2[i];
                memcpy( info->db_mk_as1 + i * DBServer.unitSize,
                        info->db_mk_as2 + i * DBServer.unitSize,DBServer.unitSize);
                info->db_mk_ba[i] = 0;
            }else{
                printf("***ERROR:db_mk_ba :%d!\n",info->db_mk_ba[i]);
                break;
            }
        }
        write(ckp_fd,info->db_mk_as2,DBServer.unitSize * db_size);
    }else if ( 2 == info->current){
        for (i = 0; i < db_size; i ++){
            if ( 0 == info->db_mk_ba[i]){
                continue;
            }else if ( 1 == info->db_mk_ba[i]){
                //info->db_mk_as2[i] = info->db_mk_as1[i];
                memcpy( info->db_mk_as2 + i * DBServer.unitSize,
                        info->db_mk_as1 + i * DBServer.unitSize,DBServer.unitSize);
                info->db_mk_ba[i] = 0;
            }else if ( 2 == info->db_mk_ba[i]){
                continue;
            }else{
                printf("***ERROR:db_mk_ba :%d!\n",info->db_mk_ba[i]);
                break;
            }
        }
        write(ckp_fd,info->db_mk_as1,DBServer.unitSize * db_size);
    }else{
        printf("ERROR:MK_CKP!\n");
    }
    fsync(ckp_fd);
    close(ckp_fd); 
    clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID*2 + 1]));   
}
void db_mk_destroy( void *mk_info)
{
    db_mk_infomation *info = mk_info;
    free(info->db_mk_as1);
    free(info->db_mk_as2);
    free(info->db_mk_ba);
    pthread_rwlock_destroy(&( info->db_rwlock));
}
