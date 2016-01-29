#include"database.h"
#include"include.h"

extern db_server DBServer;

void *database_thread(void *arg)
{
	int dbSize = ((db_thread_info *) arg)->dbSize;
	int algType = ((db_thread_info *) arg)->algType;
	pthread_barrier_t *exitBrr = ((db_thread_info *) arg)->ckpExitBrr;
	pthread_barrier_t *initBrr = ((db_thread_info *) arg)->ckpInitBrr;

	char dbLogPath[128];
	int (*db_init)(void *, int);
	void (*checkpoint)(int, void *);
	void (*db_destroy)(void *);
	void *info;



	printf("database thread start，dbSize:%d alg_type:%d,unit_size:%d,set uf:%d\n", 
		dbSize, algType,DBServer.unitSize,DBServer.updateFrequency);
	switch (algType) {
	case NAIVE_ALG:
		db_init = db_naive_init;
		checkpoint = ckp_naive;
		db_destroy = db_naive_destroy;
		info = &(DBServer.naiveInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/naive_%d_ckp_log", dbSize);
		break;
	case COPY_ON_UPDATE_ALG:
		db_init = db_cou_init;
		checkpoint = ckp_cou;
		db_destroy = db_cou_destroy;
		info = &(DBServer.couInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/cou_%d_ckp_log", dbSize);
		break;
	case ZIGZAG_ALG:
		db_init = db_zigzag_init;
		checkpoint = db_zigzag_ckp;
		db_destroy = db_zigzag_destroy;
		info = &(DBServer.zigzagInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/zigzag_%d_ckp_log", dbSize);
		break;
	case PINGPONG_ALG:
		db_init = db_pingpong_init;
		checkpoint = db_pingpong_ckp;
		db_destroy = db_pingpong_destroy;
		info = &(DBServer.pingpongInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/pingpong_%d_ckp_log", dbSize);
		break;
	case MK_ALG:
		db_init = db_mk_init;
		checkpoint = db_mk_ckp;
		db_destroy = db_mk_destroy;
		info = &(DBServer.mkInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/mk_%d_ckp_log", dbSize);
		break;
	case LL_ALG:
		db_init = db_ll_init;
		checkpoint = db_ll_ckp;
		db_destroy = db_ll_destroy;
		info = &(DBServer.llInfo);
		snprintf(dbLogPath, sizeof(dbLogPath), "./log/ll_%d_ckp_log", dbSize);
		break;
	default:
		printf("alg_type error!");
		goto DB_EXIT;
		break;
	}

	if (0 != db_init(info, dbSize)) {
		perror("db thread init error!");
		goto DB_EXIT;
	}

	pthread_rwlock_init(&(DBServer.dbStateRWLock), NULL);
	pthread_rwlock_wrlock(&DBServer.dbStateRWLock);
	DBServer.dbState = 1;
	pthread_rwlock_unlock(&DBServer.dbStateRWLock);
	
	printf("db thread init success!\n");
	pthread_barrier_wait(initBrr);



	long long timeStart;
	long long timeEnd;
	while (1) {
		timeStart = get_ntime();
		printf("time:%d\n", (int) (timeStart / 1000000000));
		checkpoint(DBServer.ckpID % 2, info);
		timeEnd = get_ntime();

		add_total_log( &DBServer, timeEnd - timeStart);
		usleep(5000000 - (timeEnd - timeStart)/1000);

		DBServer.ckpID++;
		if (DBServer.ckpID >= DBServer.ckpMaxNum) {
			
			pthread_rwlock_wrlock(&(DBServer.dbStateRWLock));
			DBServer.dbState = 0;
			pthread_rwlock_unlock(&(DBServer.dbStateRWLock));
			break;
		}
	}
	printf("\ncheckpoint finish:%d\n", DBServer.ckpID);
	pthread_barrier_wait(exitBrr);

DB_EXIT:
	printf("database thread exit\n");
	pthread_rwlock_destroy(&(DBServer.dbStateRWLock));
	db_destroy(info);
	
	pthread_exit(NULL);
}

void log_time_write(db_server *s)
{
	FILE *log_time;
	int i;
	char logName[256];
	long long timeStart;
	long long timeEnd;
	
	long long timeSum = 0;
	sprintf(logName,"./log/overhead/%d_overhead_%dk_%d_%d.log",
		DBServer.algType,DBServer.updateFrequency,
		DBServer.dbSize,DBServer.unitSize);
	if (NULL == (log_time = fopen(logName, "w"))) {
		perror("log_time fopen error,checkout if the floder is exist");
		return;
	}
	for (i = 1; i < s->ckpMaxNum; i++) {
		
		timeStart = s->ckpTimeLog[i * 2].tv_sec * 1000000000 + 
			s->ckpTimeLog[i*2].tv_nsec;
		timeEnd = s->ckpTimeLog[i * 2 + 1].tv_sec * 1000000000 + 
			s->ckpTimeLog[i*2 + 1].tv_nsec;
		timeSum += timeEnd - timeStart;
	}
	fprintf(log_time,"%lld\n",timeSum/ (s->ckpMaxNum - 1));
	fflush(log_time);
	fclose(log_time);
}

long long get_ntime(void)
{
	struct timespec timeNow;

	clock_gettime(CLOCK_MONOTONIC, &timeNow);
	return timeNow.tv_sec * 1000000000 + timeNow.tv_nsec;
}

long long get_utime(void)
{
	return get_ntime() / 1000;
}

long long get_mtime(void)
{
	return get_ntime() / 1000000;
}
