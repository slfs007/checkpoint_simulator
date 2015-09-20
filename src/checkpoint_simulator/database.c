#include"database.h"
#include"include.h"

extern db_server DBServer;

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
    
   
    ckp_db_b = ((db_thread_info *)arg)->ckp_db_barrier;
    
    printf("database thread start，db_size:%d alg_type:%d,unit_size:%d\n",db_size,alg_type,unit_size);
    switch ( alg_type)
    {
        case NAIVE_ALG:
            db_init = db_naive_init;
            checkpoint = ckp_naive;
            db_destroy = db_naive_destroy;
            info = &(DBServer.naiveInfo);
            snprintf(db_log_name,sizeof(db_log_name),"./log/naive_ckp_log");
            break;
        case COPY_ON_UPDATE_ALG:
            db_init = db_cou_init;
            checkpoint = ckp_cou;
            db_destroy = db_cou_destroy;
            info = &(DBServer.couInfo);
            snprintf(db_log_name,sizeof(db_log_name),"./log/cou_ckp_log");
            break;
        case ZIGZAG_ALG:
            db_init = db_zigzag_init;
            checkpoint = db_zigzag_ckp;
            db_destroy = db_zigzag_destroy;
            info = &(DBServer.zigzagInfo);
            snprintf(db_log_name,sizeof(db_log_name),"./log/zigzag_ckp_log");
            break;
        case PINGPONG_ALG:
            db_init = db_pingpong_init;
            checkpoint = db_pingpong_ckp;
            db_destroy = db_pingpong_destroy;
            info = &(DBServer.pingpongInfo);
            snprintf(db_log_name,sizeof(db_log_name),"./log/pingpong_ckp_log");
            break;
        case MK_ALG:
            db_init = db_mk_init;
            checkpoint = db_mk_ckp;
            db_destroy = db_mk_destroy;
            info = &(DBServer.mkInfo);
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
        
    pthread_rwlock_init(&(DBServer.dbStateRWLock),NULL);
    pthread_rwlock_wrlock(&DBServer.dbStateRWLock);
    DBServer.dbState = 1;
    pthread_rwlock_unlock(&DBServer.dbStateRWLock);
    
    printf("db thread init success!\n");
    pthread_barrier_wait( ckp_db_b);
    
    DBServer.ckpID = 0;
    ckp_max = 20;
    long long timeStart;
    long long timeEnd;
    while( 1)
    {
        timeStart = get_utime(); 
	printf("time:%d\n",(int)(timeStart/1000000));
	checkpoint(DBServer.ckpID%2, info);
	timeEnd = get_utime();
	
	usleep(5000000 - (timeEnd - timeStart));		
	
        DBServer.ckpID ++;
        if (DBServer.ckpID >= ckp_max)
        {
            pthread_rwlock_wrlock(&(DBServer.dbStateRWLock));
            DBServer.dbState = 0;
            pthread_rwlock_unlock(&(DBServer.dbStateRWLock));
            break;
        }
    }
    printf("\ncheckpoint finish:%d\n",DBServer.ckpID - 1);
    pthread_barrier_wait(brr_exit);
    
DB_EXIT:
    printf("database thread exit\n");
    pthread_rwlock_destroy(&(DBServer.dbStateRWLock));
    db_destroy(info);
    log_time_write(DBServer.ckpTimeLog,ckp_max * 2,db_log_name);
    pthread_exit(NULL);
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
long long get_ntime( void)
{
	struct timespec timeNow;
	
	clock_gettime(CLOCK_MONOTONIC, &timeNow);
	return timeNow.tv_sec * 1000000000 + timeNow.tv_nsec;
}
long long get_utime( void)
{
	return get_ntime()/1000;
}
long long get_mtime( void)
{
	return get_ntime()/1000000;
}
