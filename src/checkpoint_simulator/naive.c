#include"include.h"
#include"naive.h"
extern db_server DBServer;

int db_naive_init(void *naive_info, int db_size)
{
	db_naive_infomation *info;

	info = naive_info;
	info->db_size = db_size;

	if (NULL == (info->db_naive_AS =
		(char *) malloc(DBServer.unitSize * db_size))) {
		perror("da_navie_AS malloc error");
		return -1;
	}
	memset(info->db_naive_AS, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_naive_AS_shandow =
		(char *) malloc(DBServer.unitSize * db_size))) {
		perror("db_navie_AS_shandow malloc error");
		return -1;
	}

    info->db_naive_lock = UNLOCK;
	return 0;
}

void db_naive_destroy(void *naive_info)
{
	db_naive_infomation *info;
	info = naive_info;

	free(info->db_naive_AS);
	free(info->db_naive_AS_shandow);
}

void* naive_read(int index)
{
	void *result;
	if (index >= DBServer.dbSize) {
		index = index % DBServer.dbSize;
	}
	result = (void *) ((DBServer.naiveInfo).db_naive_AS + index * DBServer.unitSize);
	return result;
}

int naive_write(int index, void *value)
{
	if (index >= DBServer.dbSize) {
		index = index % DBServer.dbSize;
	}
    db_lock( &(DBServer.naiveInfo.db_naive_lock));
	memcpy((DBServer.naiveInfo).db_naive_AS + index * DBServer.unitSize, value, DBServer.unitSize);
    db_unlock( &(DBServer.naiveInfo.db_naive_lock));
	return 0;
}

void ckp_naive(int ckp_order, void *naive_info)
{
	int ckp_fd;
	char ckp_name[32];
	db_naive_infomation *info;
	long long timeStart;
	long long timeEnd;
	int db_size;

	info = naive_info;
	sprintf(ckp_name, "./ckp_backup/naive_%d", ckp_order);
	if (-1 == (ckp_fd = open(ckp_name, O_WRONLY | O_CREAT, 666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	timeStart = get_ntime();
    db_lock( &(DBServer.naiveInfo.db_naive_lock));
	
	memcpy(info->db_naive_AS_shandow,
		info->db_naive_AS , DBServer.unitSize * db_size);
	
    db_unlock( &(DBServer.naiveInfo.db_naive_lock));
	timeEnd = get_ntime();
	add_prepare_log(&DBServer,timeEnd - timeStart);
	
	timeStart = get_ntime();
	write(ckp_fd, info->db_naive_AS_shandow, DBServer.unitSize * db_size);
	fsync(ckp_fd);
	close(ckp_fd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
	
}
