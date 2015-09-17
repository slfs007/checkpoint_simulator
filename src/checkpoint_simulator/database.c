#include"database.h"
#include"include.h"


db_naive_infomation db_naive_info;
db_cou_infomation db_cou_info;
db_zigzag_infomation db_zigzag_info;
db_pingpong_infomation db_pingpong_info;
db_mk_infomation db_mk_info;
static int DB_SIZE;
static int UNIT_SIZE;
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
            ( char *)malloc(UNIT_SIZE * db_size))){
        perror("db_cou_primary malloc error");
        return -1;
    }
    memset(info->db_cou_primary,'S',UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_cou_shandow = 
            (char *) malloc(UNIT_SIZE * db_size))){
        perror("db_cou_shandow malloc error");
        return -1;
    }
    memset(info->db_cou_shandow,'S',UNIT_SIZE * db_size);
    
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
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    result = db_cou_info.db_cou_primary + index * UNIT_SIZE;
    return result;
}
int cou_write( int index, void *value)
{
    if ( index > DB_SIZE)
        index = index % DB_SIZE;
    
    pthread_rwlock_rdlock( &(db_cou_info.db_mutex));
    
    db_cou_info.db_cou_bitarray[index] = 1;
    memcpy(db_cou_info.db_cou_primary + index * UNIT_SIZE,value,UNIT_SIZE);
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
            memcpy(info->db_cou_shandow + i * UNIT_SIZE,
                    info->db_cou_primary + i * UNIT_SIZE,UNIT_SIZE);

        }
    }
    pthread_rwlock_unlock(&(info->db_mutex));
    
    write(ckp_fd,info->db_cou_shandow, UNIT_SIZE*db_size);
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
            ( char *)malloc( UNIT_SIZE * db_size))){
        perror("da_navie_AS malloc error");
        return -1;
    }
    
    
    memset(info->db_naive_AS,'S',UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_naive_AS_shandow = 
            ( char *)malloc( UNIT_SIZE * db_size))){
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
    int unit_size = ((db_thread_info *)arg)->unit_size;
    int ckp_max;

    pthread_barrier_t *ckp_db_b;
    
    char db_log_name[128];
    int (*db_init)(void *,int);
    void (*checkpoint)( int ,void *);
    void (*db_destroy)(void *);
    void *info;
    
    DB_SIZE = db_size;
    UNIT_SIZE = unit_size;
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d,unit_size:%d\n",db_size,alg_type,unit_size);
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
        case MK_ALG:
            db_init = db_mk_init;
            checkpoint = db_mk_ckp;
            db_destroy = db_mk_destroy;
            info = &db_mk_info;
            snprintf(db_log_name,sizeof(db_log_name), "./log/mk_ckp_log");
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
    ckp_max = 50;
    while( 1)
    {
        checkpoint(ckp_id%2, info);
        ckp_id ++;
        if (ckp_id >= ckp_max)
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
    log_time_write(ckp_time_log,ckp_max * 2,db_log_name);
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
        memcpy(info->db_naive_AS_shandow + i * UNIT_SIZE,
                info->db_naive_AS + i * UNIT_SIZE, UNIT_SIZE);
    }
   
    pthread_rwlock_unlock(&(info->write_mutex));
    
    write(ckp_fd,info->db_naive_AS_shandow,UNIT_SIZE * db_size);
    fsync(ckp_fd);
    close(ckp_fd);
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));
}
void* naive_read( int index)
{
    void *result;
    if ( index >= DB_SIZE){
        index = index % DB_SIZE;
    }
    result = ( void *) (db_naive_info.db_naive_AS + index * UNIT_SIZE);
    return result;
}
int naive_write( int index,void *value)
{
    if ( index >= DB_SIZE){
        index = index % DB_SIZE;
    }
    pthread_rwlock_rdlock(&(db_naive_info.write_mutex));
    memcpy(db_naive_info.db_naive_AS + index * UNIT_SIZE,value,UNIT_SIZE);
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
    
    if ( NULL == (info->db_zigzag_as0 = (char *)malloc(UNIT_SIZE * db_size))){
        perror("db_zigzag_as0 malloc error");
        return -1;
    }
    memset(info->db_zigzag_as0,'S',UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_zigzag_as1 = (char *)malloc(UNIT_SIZE * db_size))){
        perror("db_zigzag_sa1 malloc error");
        return -1;
    }
    memset(info->db_zigzag_as0,'S',UNIT_SIZE * db_size);
    
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
void* zigzag_read( int index)
{
    if (index > db_zigzag_info.db_size)
        index = index % db_zigzag_info.db_size;
    if (0 == db_zigzag_info.db_zigzag_mr[index]){
        return (void *)(db_zigzag_info.db_zigzag_as0 + index * UNIT_SIZE);
    }else{
        return (void *)(db_zigzag_info.db_zigzag_as1 + index * UNIT_SIZE);
    }
}
int zigzag_write( int index, void *value)
{
    if (index > db_zigzag_info.db_size)
        index = index % db_zigzag_info.db_size;
    pthread_rwlock_rdlock(&(db_zigzag_info.write_mutex));
    if (0 == db_zigzag_info.db_zigzag_mw[index]){
        //db_zigzag_info.db_zigzag_as0[index] = value;        
        memcpy(db_zigzag_info.db_zigzag_as0 + index * UNIT_SIZE,value,UNIT_SIZE);
    }else{
        //db_zigzag_info.db_zigzag_as1[index] = value;
        memcpy(db_zigzag_info.db_zigzag_as1 + index * UNIT_SIZE,value,UNIT_SIZE);
    }
    db_zigzag_info.db_zigzag_mr[index] = db_zigzag_info.db_zigzag_mw[index];
    pthread_rwlock_unlock(&(db_zigzag_info.write_mutex));
    return 0;
}

void db_zigzag_ckp( int ckp_order,void *zigzag_info)
{
    FILE *ckp_file;
    char ckp_name[128];
    int i;
    int db_size;
    db_zigzag_infomation *info;
    
    info = zigzag_info;
    sprintf(ckp_name,"./ckp_backup/zz_%d",ckp_order);
    if ( NULL == ( ckp_file = fopen(ckp_name,"w+")))
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
            fwrite( (void *)(info->db_zigzag_as1 + i * UNIT_SIZE),UNIT_SIZE,1,ckp_file);
        }else{
            fwrite( (void *)(info->db_zigzag_as0 + i * UNIT_SIZE),UNIT_SIZE,1,ckp_file);
        }
    }
    
    fflush(ckp_file);
    fclose(ckp_file); 
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
    if ( NULL == (info->db_pp_as = malloc(UNIT_SIZE * db_size))){
        perror("db_pp_as malloc error");
        return -1;
    }
    memset ( info->db_pp_as,'S',UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_pp_as_odd = malloc(UNIT_SIZE * db_size))){
        perror("db_pp_as_odd malloc error");
        return -1;
    }
    memset ( info->db_pp_as_odd,'S',UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_pp_as_even = malloc(UNIT_SIZE * db_size))){
        perror("db_pp_as_even malloc error");
        return -1;
    }
    memset ( info->db_pp_as_even,'S',UNIT_SIZE * db_size);
    
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
    
    if ( NULL == (info->db_pp_as_previous = malloc(UNIT_SIZE * db_size))){
        perror("db_pp_as_previous malloc error");
        return -1;
    }
    memset ( info->db_pp_as_even,'S',UNIT_SIZE * db_size);
    
    pthread_rwlock_init(&(info->write_mutex),NULL);
    info->current = 0;
    return 0;
}
void* pingpong_read( int index)
{
    if (index > db_pingpong_info.db_size)
        index = index % db_pingpong_info.db_size;
    return db_pingpong_info.db_pp_as+ index * UNIT_SIZE;
}
int pingpong_write( int index, void* value)
{
    if (index > db_pingpong_info.db_size)
        index = index % db_pingpong_info.db_size;
    
    //db_pingpong_info.db_pp_as[index] = value;
    memcpy(db_pingpong_info.db_pp_as + index * UNIT_SIZE,value,UNIT_SIZE);
    pthread_rwlock_rdlock(&(db_pingpong_info.write_mutex));
    if (0 == db_pingpong_info.current){
    //    db_pingpong_info.db_pp_as_odd[index] = value;
        memcpy(db_pingpong_info.db_pp_as_odd + index * UNIT_SIZE,value,UNIT_SIZE);
        db_pingpong_info.db_pp_odd_ba[index] = 1;
    }else{
    //    db_pingpong_info.db_pp_as_even[index] = value;
        memcpy(db_pingpong_info.db_pp_as_even + index * UNIT_SIZE,value,UNIT_SIZE);
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
                
                //info->db_pp_as_previous[i] = info->db_pp_as_even[i];
                memcpy(info->db_pp_as_previous + i * UNIT_SIZE,
                        info->db_pp_as_even + i * UNIT_SIZE,UNIT_SIZE);
                memset(info->db_pp_as_even + i * UNIT_SIZE,0,UNIT_SIZE);
                info->db_pp_even_ba[i] = 0;
            }
            
        }
    }else{
        for (i = 0; i < db_size; i ++){
            if (1 == info->db_pp_as_odd[i]){
                memcpy(info->db_pp_as_previous + i * UNIT_SIZE,
                        info->db_pp_as_odd + i * UNIT_SIZE,UNIT_SIZE);
                memset(info->db_pp_as_odd + i * UNIT_SIZE,0,UNIT_SIZE);
                info->db_pp_odd_ba[i] = 0;
            }
        }
    }
    //printf("WRITE:%d\n",UNIT_SIZE * db_size);
    write(ckp_fd,info->db_pp_as_previous,UNIT_SIZE * db_size);
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
int db_mk_init(void *mk_info,int db_size)
{
    db_mk_infomation *info = mk_info;
    
    info->db_size = db_size;
    
    if ( NULL == (info->db_mk_as1 = malloc( UNIT_SIZE * db_size))){
        perror("db_mk_as1 malloc error");
        return -1;
    }
    memset ( info->db_mk_as1,'S', UNIT_SIZE * db_size);
    
    if ( NULL == (info->db_mk_as2 = malloc(UNIT_SIZE * db_size))){
        perror("db_mk_as2 malloc error");
        return -1;
    }
    memset ( info->db_mk_as2,'S', UNIT_SIZE * db_size);
    
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
    if (index > db_mk_info.db_size)
        index = index % db_mk_info.db_size;
    if (1 == db_mk_info.current){
        return db_mk_info.db_mk_as1 + index * UNIT_SIZE;
    }else if(2 == db_mk_info.current){
        return db_mk_info.db_mk_as2 + index * UNIT_SIZE;
    }else{
        printf("ERROR:MK_READ!\n");
    }
    return NULL;
}
int mk_write( int index, void* value)
{
    if (index > db_mk_info.db_size)
        index = index % db_mk_info.db_size;
    
    pthread_rwlock_rdlock(&(db_mk_info.db_rwlock));
    if ( 0 == db_mk_info.lock){
        
        //db_mk_info.db_mk_as1[index] = value;
        memcpy(db_mk_info.db_mk_as1 + index * UNIT_SIZE,value,UNIT_SIZE);
        //db_mk_info.db_mk_as2[index] = value;
        memcpy(db_mk_info.db_mk_as2 + index * UNIT_SIZE,value,UNIT_SIZE);
        
        db_mk_info.db_mk_ba[index] = 0;
        pthread_rwlock_unlock(&(db_mk_info.db_rwlock));    
        return 0;
    }
    //lock
    if ( 1 == db_mk_info.current){
        
        //db_mk_info.db_mk_as1[index] = value;
        memcpy(db_mk_info.db_mk_as1 + index * UNIT_SIZE,value,UNIT_SIZE);
        
        db_mk_info.db_mk_ba[index] = 1;
    }else if (2 == db_mk_info.current){
        
        //db_mk_info.db_mk_as2[index] = value;
        memcpy(db_mk_info.db_mk_as2 + index * UNIT_SIZE,value,UNIT_SIZE);
        
        db_mk_info.db_mk_ba[index] = 2;
    }else{
        
        printf("ERROR:MK_WRITE\n");
    }
    pthread_rwlock_unlock(&(db_mk_info.db_rwlock));
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
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2]));
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
                memcpy( info->db_mk_as1 + i * UNIT_SIZE,
                        info->db_mk_as2 + i * UNIT_SIZE,UNIT_SIZE);
                info->db_mk_ba[i] = 0;
            }else{
                printf("***ERROR:db_mk_ba :%d!\n",info->db_mk_ba[i]);
                break;
            }
        }
        write(ckp_fd,info->db_mk_as2,UNIT_SIZE * db_size);
    }else if ( 2 == info->current){
        for (i = 0; i < db_size; i ++){
            if ( 0 == info->db_mk_ba[i]){
                continue;
            }else if ( 1 == info->db_mk_ba[i]){
                //info->db_mk_as2[i] = info->db_mk_as1[i];
                memcpy( info->db_mk_as2 + i * UNIT_SIZE,
                        info->db_mk_as1 + i * UNIT_SIZE,UNIT_SIZE);
                info->db_mk_ba[i] = 0;
            }else if ( 2 == info->db_mk_ba[i]){
                continue;
            }else{
                printf("***ERROR:db_mk_ba :%d!\n",info->db_mk_ba[i]);
                break;
            }
        }
        write(ckp_fd,info->db_mk_as1,UNIT_SIZE * db_size);
    }else{
        printf("ERROR:MK_CKP!\n");
    }
    fsync(ckp_fd);
    close(ckp_fd); 
    clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2 + 1]));   
}
void db_mk_destroy( void *mk_info)
{
    db_mk_infomation *info = mk_info;
    free(info->db_mk_as1);
    free(info->db_mk_as2);
    free(info->db_mk_ba);
    pthread_rwlock_destroy(&( info->db_rwlock));
}