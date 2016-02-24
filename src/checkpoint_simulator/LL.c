#include"LL.h"
extern db_server DBServer;

int db_ll_init(void *ll_info, int db_size)
{
	db_ll_infomation *info = ll_info;

	info->db_size = db_size;

	if (NULL == (info->db_ll_as0 = malloc(DBServer.unitSize * db_size))) {
		perror("db_ll_as0 malloc error");
		return -1;
	}
	memset(info->db_ll_as0, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_ll_as1 = malloc(DBServer.unitSize * db_size))) {
		perror("db_ll_as1 malloc error");
		return -1;
	}
	memset(info->db_ll_as1, 'S', DBServer.unitSize * db_size);
	if (NULL == (info->db_ll_prev = malloc(DBServer.unitSize * db_size))) {
		perror("db_ll_prev malloc error");
		return -1;
	}
	memset(info->db_ll_prev, 'S', DBServer.unitSize * db_size);
	if (NULL == (info->db_ll_as0_ba = malloc(db_size))) {
		perror("db_ll_as0_ba malloc error");
		return -1;
	}
	memset(info->db_ll_as0_ba, 0, db_size);
	if (NULL == (info->db_ll_as1_ba = malloc(db_size))) {
		perror("db_ll_as1_ba malloc error");
		return -1;
	}
	memset(info->db_ll_as1_ba, 0, db_size);
	if (NULL == (info->db_ll_mr_ba = malloc(db_size))) {
		perror("db_ll_as1_ba malloc error");
		return -1;
	}
	memset(info->db_ll_mr_ba, 0, db_size);
	pthread_rwlock_init(&(info->db_rwlock), NULL);
	info->current = 0;
	return 0;

}

void* ll_read(int index)
{
	if (index > (DBServer.llInfo).db_size)
		index = index % (DBServer.llInfo).db_size;
	if (1 == (DBServer.llInfo).current) {
		return (DBServer.llInfo).db_ll_as1 + index * DBServer.unitSize;
	} else {
		return( DBServer.llInfo).db_ll_as0 + index * DBServer.unitSize;
	} 
	return NULL;
}

int ll_write(int index, void* value)
{
	if (index > (DBServer.llInfo).db_size)
		index = index % (DBServer.llInfo).db_size;

	pthread_rwlock_rdlock(&((DBServer.llInfo).db_rwlock));

	if (1 == (DBServer.llInfo).current) {
		
		memcpy((DBServer.llInfo).db_ll_as1 + index * DBServer.unitSize , value, DBServer.unitSize);
		(DBServer.llInfo).db_ll_as1_ba[index] = 1;
	} else {
		
		memcpy((DBServer.llInfo).db_ll_as0 + index * DBServer.unitSize , value, DBServer.unitSize);
		(DBServer.llInfo).db_ll_as0_ba[index] = 1;
	}
	pthread_rwlock_unlock(&((DBServer.llInfo).db_rwlock));
	return 0;
}

void db_ll_ckp(int ckp_order, void *ll_info)
{
	int ckp_fd;
	char ckp_name[32];
	int i;
	int db_size;
	db_ll_infomation *info;
	char *currentBackup;
	unsigned char *currentBA;
	long long timeStart;
	long long timeEnd;
	
	info = ll_info;
	sprintf(ckp_name, "./ckp_backup/pp_%d", ckp_order);
	if (-1 == (ckp_fd = open(ckp_name, O_WRONLY | O_CREAT, 666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	
	timeStart = get_ntime();
	pthread_rwlock_wrlock(&(info->db_rwlock));
	//prepare for checkpoint
	info->current = !(info->current);
	pthread_rwlock_unlock(&(info->db_rwlock));
	if (0 == info->current) {
		currentBackup = info->db_ll_as1;
		currentBA = info->db_ll_as1_ba;
	} else {
		currentBackup = info->db_ll_as0;
		currentBA = info->db_ll_as0_ba;
	}
	for (i = 0; i < db_size; i++) {
		if (1 == currentBA[i]) {
			//info->db_pp_as_previous[i] = info->db_pp_as_even[i];
			memcpy(info->db_ll_prev + i * DBServer.unitSize,
				currentBackup + i * DBServer.unitSize, DBServer.unitSize);
			currentBA[i] = 0;
		}
	}
	timeEnd = get_ntime();
	add_prepare_log(&DBServer,timeEnd - timeStart);
	timeStart = get_ntime();
	//write to disk
	write(ckp_fd, info->db_ll_prev, DBServer.unitSize * db_size);
	fsync(ckp_fd);
	close(ckp_fd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
}

void db_ll_destroy(void *ll_info)
{
	db_ll_infomation *info = ll_info;
	free(info->db_ll_as1);
	free(info->db_ll_as0);
	free(info->db_ll_prev);
	free(info->db_ll_as1_ba);
	free(info->db_ll_as0_ba);
	free(info->db_ll_mr_ba);
	
	pthread_rwlock_destroy(&(info->db_rwlock));
}

