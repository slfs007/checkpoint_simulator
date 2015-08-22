#include "db_zigzag.h" 
#include "include.h"

extern struct timespec *ckp_time_log;// 临时设置的全局变量
extern int ckp_id;

extern db_zigzag_infomation db_zigzag_info;

int db_zigzag_init(void *zigzag_info,int db_size)
{
    db_zigzag_infomation *info;
    
    info = zigzag_info;
    
    info->db_size = db_size;
    
    if ( NULL == (info->db_zigzag_as0 = (int *)malloc(sizeof(int) * db_size))){
        perror("db_zigzag_as0 malloc error");
        return -1;
    }
    memset(info->db_zigzag_as0,'S',sizeof(int) * db_size);
    
    if ( NULL == (info->db_zigzag_as1 = (int *)malloc(sizeof(int) * db_size))){
        perror("db_zigzag_sa1 malloc error");
        return -1;
    }
    memset(info->db_zigzag_as0,'S',sizeof(int) * db_size);
    
    if ( NULL == (info->db_zigzag_mr = (unsigned char *)malloc(db_size))){
        perror("db_zigzag_mr malloc error");
        return -1;
    }
    memset(info->db_zigzag_mr,0,db_size);
    
    if ( NULL == (info->db_zigzag_mw = (unsigned char *)malloc(db_size))){
        perror("db_zigzag_mw malloc error");
        return -1;
    }
    memset (info->db_zigzag_mw,1,db_size);
    pthread_rwlock_init( &(info->write_mutex),NULL);
    return 0;
}
int zigzag_read( int index)
{
    if (index > db_zigzag_info.db_size)
        index = index % db_zigzag_info.db_size;
    if (0 == db_zigzag_info.db_zigzag_mr[index]){
        return db_zigzag_info.db_zigzag_as0[index];
    }else{
        return db_zigzag_info.db_zigzag_as1[index];
    }
}
int zigzag_write( int index, int value)
{
    if (index > db_zigzag_info.db_size)
        index = index % db_zigzag_info.db_size;
    pthread_rwlock_rdlock(&(db_zigzag_info.write_mutex));
    if (0 == db_zigzag_info.db_zigzag_mw[index]){
        db_zigzag_info.db_zigzag_as0[index] = value;        
    }else{
        db_zigzag_info.db_zigzag_as1[index] = value;
    }
    db_zigzag_info.db_zigzag_mr[index] = db_zigzag_info.db_zigzag_mw[index];
    pthread_rwlock_unlock(&(db_zigzag_info.write_mutex));
    return 0;
}

void db_zigzag_ckp( int ckp_order,void *zigzag_info)
{
    FILE *ckp_file;
    char ckp_name[32];
    int i;
    int db_size;
    db_zigzag_infomation *info;
    
    info = zigzag_info;
    sprintf(ckp_name,"./ckp_backup/cou_%d",ckp_order);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;
    pthread_rwlock_wrlock(&(info->write_mutex));
    //prepare for checkpoint
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
    for (i = 0; i < db_size; i ++){
        info->db_zigzag_mw[i] = !(info->db_zigzag_mr[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
    pthread_rwlock_unlock(&(info->write_mutex));
    //write to disk
    for (i = 0;i < db_size; i ++){
        if (0 == info->db_zigzag_mw[i]){
            fwrite(info->db_zigzag_as1,sizeof( int),1,ckp_file);
        }else{
            fwrite(info->db_zigzag_as0,sizeof( int),1,ckp_file);
        }
    }
    fflush(ckp_file);
    fclose(ckp_file); 
}
void db_zigzag_destroy( void *zigzag_info)
{
    db_zigzag_infomation *info;
    info = zigzag_info;
    
    free(info->db_zigzag_as0);
    free(info->db_zigzag_as1);
    free(info->db_zigzag_mr);
    free(info->db_zigzag_mw);
    pthread_rwlock_destroy( &(info->write_mutex));
}
