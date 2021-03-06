#include"update.h"
#include"include.h"


void* (*db_read)(int index);
int (*db_write)(int index, void* value);

extern pthread_barrier_t brr_exit;

void *update_thread(void *arg)
{

	int alg_type = ((update_thread_info *) arg) ->alg_type;
	int *random_buffer = ((update_thread_info *) arg) ->random_buffer;
	int random_buffer_size = ((update_thread_info *) arg) ->random_buffer_size;
	pthread_barrier_t *update_brr_init = ((update_thread_info *) arg)->update_brr_init;
	pthread_barrier_t *brr_exit = ((update_thread_info *) arg)->brr_exit;
	int pthread_id = ((update_thread_info *) arg)->pthread_id;
	int update_frequency = ((update_thread_info *) arg)->update_frequency;
	char log_name[128];



	switch (alg_type) {
	case NAIVE_ALG:
		db_write = naive_write;
		db_read = naive_read;
		//    snprintf(log_name,sizeof(log_name),"./log/naive_update_log_%d",pthread_id);
		break;
	case COPY_ON_UPDATE_ALG:
		db_write = cou_write;
		db_read = cou_read;
		//    snprintf(log_name,sizeof(log_name),"./log/cou_update_log_%d",pthread_id);
		break;
	case ZIGZAG_ALG:
		db_write = zigzag_write;
		db_read = zigzag_read;
		//    snprintf(log_name,sizeof(log_name),"./log/zigzag_update_log_%d",pthread_id);

		break;
	case PINGPONG_ALG:
		db_write = pingpong_write;
		db_read = pingpong_read;
		//    snprintf(log_name,sizeof(log_name),"./log/pingpong_update_log_%d",pthread_id);

		break;
	case MK_ALG:
		db_write = mk_write;
		db_read = mk_read;
		//    snprintf(log_name,sizeof(log_name),"./log/mk_update_log_%d",pthread_id);
		break;
	default:
		perror("alg_type error");
		break;
	}
	sprintf(log_name, "./log/latency/%d_latency_%dk_%d_%d_%d", DBServer.algType,
		DBServer.updateFrequency / 1000, DBServer.dbSize, DBServer.unitSize,
		pthread_id);
	pthread_barrier_wait(update_brr_init);
	random_update_db(random_buffer, random_buffer_size, log_name, update_frequency);

	pthread_barrier_wait(brr_exit);

	pthread_exit(NULL);
}

int execute_update(int *random_buf, int buf_size, int times, FILE *log)
{
	int i;
	int buf;


	long long timeStartNs;
	long long timeEndNs;
	
	for (i = 0; i < times; i++) {
		timeStartNs = get_ntime();
		
		pthread_rwlock_rdlock(&(DBServer.dbStateRWLock));
		if (1 != DBServer.dbState) {
			printf("update thread prepare to exit\n");
			return -1;
		}
		buf = random_buf[i % buf_size];
		db_write(buf % DBServer.dbSize, random_buf);
		pthread_rwlock_unlock(&(DBServer.dbStateRWLock));

		timeEndNs = get_ntime();
		fprintf(log, "%d,%lld\n",timeStartNs / 1000000,timeEndNs - timeStartNs);
	}
	return 0;
}

int random_update_db(int *random_buf, int buf_size, char *log_name, int uf)
{
	long long i;
	int tick;
	FILE *logFile;

	long int timeNowUs;
	long int timeStartUs;
	long int timeTickUs;
	long int timeBeginUs;
	long int timeDiffUs;

	logFile = fopen(log_name, "w+");

	tick = 0;
	//   clock_gettime(CLOCK_MONOTONIC, &time_start);
	//   time_begin_us = time_start.tv_sec * 1000000 + time_start.tv_nsec / 1000;
	timeBeginUs = get_utime();
	while (1) {
		//      clock_gettime(CLOCK_MONOTONIC, &time_start);
		//      time_start_us = time_start.tv_sec * 1000000 + time_start.tv_nsec / 1000;
		timeStartUs = get_utime();
		for (i = 0; i < 1000; i++) {
			if (-1 == execute_update(random_buf, buf_size, uf / 1000, logFile)) {
				//clock_gettime(CLOCK_MONOTONIC, &time_now);
				timeNowUs = get_utime();
				goto EXIT;
			}
			//clock_gettime(CLOCK_MONOTONIC, &time_now);
			//time_now_us = time_now.tv_sec * 1000000 + time_now.tv_nsec / 1000;

			timeTickUs = timeStartUs + i * 1000;
			timeNowUs = get_utime();
			if (timeNowUs < timeTickUs) {
				usleep(timeTickUs - timeNowUs);
			}
		}
		tick++;
	}
	//clock_gettime(CLOCK_MONOTONIC, &(ckp_time_log[ckp_id*2])); 
EXIT:
	fclose(logFile);
	tick = tick * uf + i * (uf / 1000);
	//time_now_us = time_now.tv_sec * 1000000 + time_now.tv_nsec / 1000;

	timeDiffUs = (timeNowUs - timeBeginUs) / 1000000;

	printf("set uf:%d,real uf:%ld\n", uf, timeDiffUs == 0 ? 0 : tick / timeDiffUs);
	return 0;
}