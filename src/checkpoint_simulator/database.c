#include"database.h"
#include"include.h"


db_naive_infomation db_naive_info;
db_cou_infomation db_cou_info;
db_zigzag_infomation db_zigzag_info;
db_pingpong_infomation db_pingpong_info;

static int DB_SIZE;

struct timespec ckp_time_log[2000];
int ckp_id;
    
int DB_STATE;
pthread_rwlock_t DB_STATE_rw_lock;

int db_cou_init(void *cou_info,int db_size)
{
    db_cou_infomation *info;
    
    info = cou_info;
    info->db_size = db_size;
    
    if ( NULL == (info->db_cou_primary = 
            ( int *)malloc(sizeof(int) * db_size))){
        perror("db_cou_primary malloc error");
        return -1;
    }
    memset(info->db_cou_primary,'S',sizeof( int) * db_size);
    
    if ( NULL == (info->db_cou_shandow = 
            (int *) malloc(sizeof(int) * db_size))){
        perror("db_cou_shandow malloc error");
        return -1;
    }
    memset(info->db_cou_shandow,'S',sizeof( int) * db_size);
    
    if ( NULL == ( info->db_cou_bitarray = 
            (unsigned char *)malloc(db_size))){
        perror("db_cou_bitarray malloc error");
        return -1;
    }
    memset(info->db_cou_bitarray,0, db_size);
    
    pthread_rwlock_init(&(info->db_mutex), NULL);
    
    return 0;
}
int cou_read( int index)
{
    int result;
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    result = db_cou_info.db_cou_primary[index];
    return result;
}
int cou_write( int index, int value)
{
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    
    pthread_rwlock_rdlock( &(db_cou_info.db_mutex));
    db_cou_info.db_cou_bitarray[index] = 1;
    db_cou_info.db_cou_primary[index] = value;
    pthread_rwlock_unlock( &(db_cou_info.db_mutex));
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
    
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
    for (i = 0; i < db_size; i ++){
        if ( 1 == info->db_cou_bitarray[i]){
            info->db_cou_bitarray[i] = 0;
            memcpy(&(info->db_cou_shandow[i]),
                    &(info->db_cou_primary[i]),sizeof(int));

        }
    }
    pthread_rwlock_unlock(&(info->db_mutex));
    
    write(ckp_fd,info->db_cou_shandow,sizeof( int)*db_size);
    fsync(ckp_fd);
    close(ckp_fd);
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
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
int db_naive_init(void *naive_info,int db_size)
{
    db_naive_infomation *info;
    
    info = naive_info;
    info->db_size = db_size;
    if ( NULL == (info->db_naive_AS = 
            ( int *)malloc( sizeof(int) * db_size))){
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    
    memset(info->db_naive_AS,'S',sizeof( int) * db_size);
    
    if ( NULL == (info->db_naive_AS_shandow = 
            ( int *)malloc( sizeof(int) * db_size))){
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
void *database_thread(void *arg)
{
    int db_size = ((db_thread_info *)arg)->db_size;
    int alg_type = ((db_thread_info *)arg)->alg_type;
    pthread_barrier_t *brr_exit = ((db_thread_info *)arg)->brr_db_exit;
    int ckp_num;

    pthread_barrier_t *ckp_db_b;
    
    char db_log_name[128];
    int (*db_init)(void *,int);
    void (*checkpoint)( int ,void *);
    void (*db_destroy)(void *);
    void *info;
    
    DB_SIZE = db_size;
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d\n",db_size,alg_type);
    switch ( alg_type)
    {
        case NAIVE_ALG:
            db_init = db_naive_init;
            checkpoint = ckp_naive;
            db_destroy = db_naive_destroy;
            info = &db_naive_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/naive_ckp_log");
            break;
        case COPY_ON_UPDATE_ALG:
            db_init = db_cou_init;
            checkpoint = ckp_cou;
            db_destroy = db_cou_destroy;
            info = &db_cou_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/cou_ckp_log");
            break;
        case ZIGZAG_ALG:
            db_init = db_zigzag_init;
            checkpoint = db_zigzag_ckp;
            db_destroy = db_zigzag_destroy;
            info = &db_zigzag_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/zigzag_ckp_log");
            break;
        case PINGPONG_ALG:
            db_init = db_pingpong_init;
            checkpoint = db_pingpong_ckp;
            db_destroy = db_pingpong_destroy;
            info = &db_pingpong_info;
            snprintf(db_log_name,sizeof(db_log_name),"./log/pingpong_ckp_log");
            break;
        default:
            printf("alg_type error!");
            goto DB_EXIT;
            break;
    }
    
    if ( 0 != db_init(info,db_size)){
        perror("db thread init error!");
        goto DB_EXIT;
    }
        
    pthread_rwlock_init(&DB_STATE_rw_lock,NULL);
    pthread_rwlock_wrlock(&DB_STATE_rw_lock);
    DB_STATE = 1;
    pthread_rwlock_unlock(&DB_STATE_rw_lock);
    
    printf("db thread init success!\n");
    pthread_barrier_wait( ckp_db_b);
    
    ckp_id = 0;
    ckp_num = 50;
    while( 1)
    {
        
        //printf("%d.",ckp_id);
        checkpoint(ckp_id%1, info);
        
        ckp_id ++;
        if (ckp_id >= ckp_num)
        {
            pthread_rwlock_wrlock(&DB_STATE_rw_lock);
            DB_STATE = 0;
            pthread_rwlock_unlock(&DB_STATE_rw_lock);
            break;
        }
    }
    printf("\ncheckpoint finish:%d\n",ckp_id - 1);
    pthread_barrier_wait(brr_exit);
    
DB_EXIT:
    printf("database thread exit\n");
    pthread_rwlock_destroy(&DB_STATE_rw_lock);
    db_destroy(info);
    log_time_write(ckp_time_log,ckp_num * 2,db_log_name);
    pthread_exit(NULL);
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
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT | O_SYNC,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;

    pthread_rwlock_wrlock(& (info->write_mutex));
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
    for ( i = 0; i < db_size; i ++){
        memcpy(&(info->db_naive_AS_shandow[i]),
                &(info->db_naive_AS[i]),sizeof(int));
    }
   
    pthread_rwlock_unlock(&(info->write_mutex));
    
    write(ckp_fd,info->db_naive_AS_shandow,sizeof( int) * db_size);
    fsync(ckp_fd);
    close(ckp_fd);
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
}
int naive_read( int index)
{
    int result;
    if ( index >= DB_SIZE){
        index = index % DB_SIZE;
    }
    result =  db_naive_info.db_naive_AS[index];
    return result;
}
int naive_write( int index,int value)
{
    if ( index >= DB_SIZE){
        index = index % DB_SIZE;
    }
    pthread_rwlock_rdlock(&(db_naive_info.write_mutex));
    db_naive_info.db_naive_AS[index] = value;
    pthread_rwlock_unlock(&(db_naive_info.write_mutex));
    return 0;
}
void log_time_write( struct timespec *log,int log_size,char *log_name)
{
    FILE *log_time;
    int i;
  
   if ( NULL == (log_time = fopen(log_name,"w"))){
        perror("log_time fopen error,checkout if the floder is exist");
        return;
    }
    for (i = 0; i < log_size; i ++)
    {
        fprintf(log_time,"%ld,%ld\n",log[i].tv_sec,log[i].tv_nsec);
    }
    fflush(log_time);
    fclose(log_time);
}

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
    int ckp_fd;
    char ckp_name[128];
    int i;
    int db_size;
    db_zigzag_infomation *info;
    
    info = zigzag_info;
    sprintf(ckp_name,"./ckp_backup/cou_%d",ckp_order);
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;
    pthread_rwlock_wrlock(&(info->write_mutex));
    
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
    //prepare for checkpoint
    for (i = 0; i < db_size; i ++){
        info->db_zigzag_mw[i] = !(info->db_zigzag_mr[i]);
    }
    pthread_rwlock_unlock(&(info->write_mutex));
    //write to disk
    for (i = 0;i < db_size; i ++){
        if (0 == info->db_zigzag_mw[i]){
            write(ckp_fd,&(info->db_zigzag_as1[i]),sizeof( int));
        }else{
            write(ckp_fd,&(info->db_zigzag_as0[i]),sizeof( int));
        }
    }
    fsync(ckp_fd);
    close(ckp_fd); 
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
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
int db_pingpong_init(void *pp_info,int db_size)
{
    db_pingpong_infomation *info;
    
    info = pp_info;
    info->db_size = db_size;
    if ( NULL == (info->db_pp_as = malloc(sizeof(int) * db_size))){
        perror("db_pp_as malloc error");
        return -1;
    }
    memset ( info->db_pp_as,'S',sizeof(int) * db_size);
    
    if ( NULL == (info->db_pp_as_odd = malloc(sizeof(int) * db_size))){
        perror("db_pp_as_odd malloc error");
        return -1;
    }
    memset ( info->db_pp_as_odd,'S',sizeof(int) * db_size);
    
    if ( NULL == (info->db_pp_as_even = malloc(sizeof(int) * db_size))){
        perror("db_pp_as_even malloc error");
        return -1;
    }
    memset ( info->db_pp_as_even,'S',sizeof(int) * db_size);
    
    if ( NULL == (info->db_pp_odd_ba = malloc(db_size))){
        perror("db_pp_current_odd malloc error");
        return -1;
    }
    memset(info->db_pp_odd_ba,0,db_size);
    
    if ( NULL == (info->db_pp_even_ba = malloc(db_size))){
        perror("db_pp_previous_ba malloc error");
        return -1;
    }
    memset(info->db_pp_even_ba,1,db_size);
    
    if ( NULL == (info->db_pp_as_previous = malloc(db_size * sizeof(int)))){
        perror("db_pp_as_previous malloc error");
        return -1;
    }
    memset ( info->db_pp_as_even,'S',sizeof(int) * db_size);
    
    pthread_rwlock_init(&(info->write_mutex),NULL);
    info->current = 0;
    return 0;
}
int pingpong_read( int index)
{
    if (index > db_pingpong_info.db_size)
        index = index % db_pingpong_info.db_size;
    return db_pingpong_info.db_pp_as[index];
}
int pingpong_write( int index, int value)
{
    if (index > db_pingpong_info.db_size)
        index = index % db_pingpong_info.db_size;
    
    db_pingpong_info.db_pp_as[index] = value;
    
    pthread_rwlock_rdlock(&(db_pingpong_info.write_mutex));
    if (0 == db_pingpong_info.current){
        db_pingpong_info.db_pp_as_odd[index] = value;
        db_pingpong_info.db_pp_odd_ba[index] = 1;
    }else{
        db_pingpong_info.db_pp_as_even[index] = value;
        db_pingpong_info.db_pp_even_ba[index] = 1;
    }
    pthread_rwlock_unlock(&(db_pingpong_info.write_mutex));
    return 0;
}
void db_pingpong_ckp( int ckp_order,void *pp_info)
{
    int ckp_fd;
    char ckp_name[32];
    int i;
    int db_size;
    db_pingpong_infomation *info;
    
    info = pp_info;
    sprintf(ckp_name,"./ckp_backup/pp_%d",ckp_order);
    if ( -1 == ( ckp_fd = open(ckp_name,O_WRONLY | O_CREAT | O_SYNC,666)))
    {
        perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
        return;
    }
    db_size = info->db_size;

    pthread_rwlock_wrlock(&(info->write_mutex));
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
    //prepare for checkpoint
    info->current = ! (info->current);
    pthread_rwlock_unlock(&(info->write_mutex));
    //write to disk
    if (0 == info->current){
        for (i = 0; i < db_size; i ++){
            if (1 == info->db_pp_as_even[i]){
                info->db_pp_as_previous[i] = info->db_pp_as_even[i];
                info->db_pp_as_even[i] = 0;
                info->db_pp_even_ba[i] = 0;
            }
            
        }
    }else{
        for (i = 0; i < db_size; i ++){
            if (1 == info->db_pp_as_odd[i]){
                info->db_pp_as_previous[i] = info->db_pp_as_odd[i];
                info->db_pp_as_odd[i] = 0;
                info->db_pp_odd_ba[i] = 0;
            }
        }
    }
    write(ckp_fd,info->db_pp_as_previous,sizeof( int) * db_size);
    fsync(ckp_fd);
    close(ckp_fd); 
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));   
    
}
void db_pingpong_destroy( void *pp_info)
{
    db_pingpong_infomation *info;
    
    info = pp_info;
    free(info->db_pp_as);
    free(info->db_pp_as_even);
    free(info->db_pp_as_odd);
    free(info->db_pp_as_previous);
    free(info->db_pp_even_ba);
    free(info->db_pp_odd_ba);
    pthread_rwlock_destroy(&(info->write_mutex));
}