#include"include.h"


db_server DBServer;
int db_thread_start(pthread_t *db_thread_id, pthread_barrier_t *brr_exit, db_server *dbs);
int update_thread_start(pthread_t *update_thread_id_array[],
	pthread_barrier_t *brr_exit,
	db_server *dbs);
void write_overhead_log(db_server *s,const char *filePath);
int randomfile_init(FILE *rf,int *rbuf,int rbufSize);
int main(int argc, char *argv[])
{
	int i;
	FILE *rf;
	pthread_t *update_thread_array;
	pthread_t db_thread_id;
	pthread_barrier_t brr_exit;
	char logName[128];
	if (argc != 7) {
		perror("usage:./ckp_cimulator [update thread number] [unit num] "
			"[algorithm type:0-navie 1-copy on update 2-zigzag 3-pingpong] "
			"[random file name] [update frequency (k/sec)]"
			"[unit size]");
	}

	DBServer.updateThrNum = atoi(argv[1]);
	DBServer.dbSize = atoi(argv[2]);
	DBServer.algType = atoi(argv[3]);
	DBServer.updateFrequency = atoi(argv[5]);
	DBServer.unitSize = atoi(argv[6]);
	DBServer.updateFrequency *= 1000;
	DBServer.ckpID = 0;
	DBServer.dbState = 0;
	DBServer.ckpMaxNum = 10;
	DBServer.ckpOverheadLog = malloc( sizeof(long long) * DBServer.ckpMaxNum);
	
	if (NULL == (rf = fopen(argv[4], "r"))) {
		perror("random file open error!\n");
		return -1;
	}
	
	DBServer.rfBufSize = DBServer.updateFrequency * 10;
	DBServer.rfBuf = (int *) malloc(DBServer.rfBufSize * sizeof(int));
	if (DBServer.rfBufSize != randomfile_init(rf,DBServer.rfBuf,DBServer.rfBufSize)){
		perror("random file init error\n");
		return -1;
	}
	fclose(rf);

	pthread_barrier_init(&brr_exit, NULL, DBServer.updateThrNum + 1);
	if (0 != db_thread_start(&db_thread_id, &brr_exit, &DBServer)) {
		perror("db thread start fail!");
		exit(1);
	}
	if (0 != update_thread_start(&update_thread_array, &brr_exit,
		&DBServer)) {
		return -3;
	}
	//wait for quit
	pthread_join(db_thread_id, NULL);
	for (i = 0; i < DBServer.updateThrNum; i++) {
		pthread_join(update_thread_array[i], NULL);
		printf("update thread %d exit!\n", i);
	}
	free(update_thread_array);
	pthread_barrier_destroy(&brr_exit);
	sprintf(logName,"./log/overhead/%d_overhead_%dk_%d_%d.log",
		DBServer.algType,DBServer.updateFrequency,
		DBServer.dbSize,DBServer.unitSize);

	exit(1);
}
int randomfile_init(FILE *rf,int *rbuf,int rbufSize)
{
	int i;
	
	for (i = 0; i < rbufSize; i ++){
		fscanf(rf,"%d\n",rbuf + i);
	}
	
	return i;
	
}
void add_overhead_log(db_server *s,long long ns)
{
	s->ckpOverheadLog[s->ckpID] = ns;
}
void write_overhead_log(db_server *s,const char *filePath)
{
	FILE *logFile;
	int i;
	logFile = fopen(filePath,"w");
	
	for (i = 0; i < s->ckpID;i ++ ){
		
		fprintf(logFile,"%lld\n",s->ckpOverheadLog[i]);
	}
	fflush( logFile);
	fclose( logFile);
}
int db_thread_start(pthread_t *db_thread_id, pthread_barrier_t *brr_exit, db_server *dbs)
{
	db_thread_info dbInfo;
	pthread_barrier_t brrDBInit;

	pthread_barrier_init(&brrDBInit, NULL, 2);
	dbInfo.algType = dbs->algType;
	dbInfo.dbSize = dbs->dbSize;
	dbInfo.ckpInitBrr = &brrDBInit;
	dbInfo.ckpExitBrr = brr_exit;

	if (0 != pthread_create(db_thread_id, NULL, database_thread, &dbInfo)) {
		perror("database thread create error!");
		return -1;
	}

	pthread_barrier_wait(&brrDBInit);
	pthread_barrier_destroy(&brrDBInit);

	return 0;
}

int update_thread_start(pthread_t *update_thread_id_array[],
	pthread_barrier_t *brr_exit,
	db_server *dbs)
{

	int i;
	update_thread_info update_info;
	pthread_barrier_t update_brr_init;

	update_info.alg_type = dbs->algType;
	update_info.db_size = dbs->dbSize;
	update_info.random_buffer = dbs->rfBuf;
	update_info.random_buffer_size = dbs->rfBufSize;
	pthread_barrier_init(&update_brr_init, NULL, dbs->updateThrNum + 1);
	update_info.update_brr_init = &update_brr_init;
	update_info.brr_exit = brr_exit;
	update_info.update_frequency = dbs->updateFrequency;

	if (NULL == ((*update_thread_id_array)
		= (pthread_t *) malloc(sizeof(pthread_t) * dbs->updateThrNum))) {
		perror("update thread array malloc error");
	}
	for (i = 0; i < dbs->updateThrNum; i++) {
		update_info.pthread_id = i;
		if (0 != pthread_create(&((*update_thread_id_array)[i]),
			NULL, update_thread, &update_info)) {
			printf("update thread %d create error", i);
		} else {
			printf("update thread %d create success\n", i);
		}
	}
	pthread_barrier_wait(&update_brr_init);
	pthread_barrier_destroy(&update_brr_init);
	return 0;

}
