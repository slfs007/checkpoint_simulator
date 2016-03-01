#include"pingpong.h"
extern db_server DBServer;

int db_pingpong_init(void *pp_info, int db_size)
{
	db_pingpong_infomation *info;

	info = pp_info;
	info->db_size = db_size;
	if (NULL == (info->db_pp_as = malloc(DBServer.unitSize * db_size))) {
		perror("db_pp_as malloc error");
		return -1;
	}
	memset(info->db_pp_as, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_pp_as_odd = malloc(DBServer.unitSize * db_size))) {
		perror("db_pp_as_odd malloc error");
		return -1;
	}
	memset(info->db_pp_as_odd, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_pp_as_even = malloc(DBServer.unitSize * db_size))) {
		perror("db_pp_as_even malloc error");
		return -1;
	}
	memset(info->db_pp_as_even, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_pp_odd_ba = malloc(db_size))) {
		perror("db_pp_current_odd malloc error");
		return -1;
	}
	memset(info->db_pp_odd_ba, 0, db_size);

	if (NULL == (info->db_pp_even_ba = malloc(db_size))) {
		perror("db_pp_previous_ba malloc error");
		return -1;
	}
	memset(info->db_pp_even_ba, 1, db_size);

	if (NULL == (info->db_pp_as_previous = malloc(DBServer.unitSize * db_size))) {
		perror("db_pp_as_previous malloc error");
		return -1;
	}
	memset(info->db_pp_as_even, 'S', DBServer.unitSize * db_size);

    info->db_pp_lock = UNLOCK;
	info->current = 0;
	return 0;
}

void* pingpong_read(int index)
{
	if (index > (DBServer.pingpongInfo).db_size)
		index = index % (DBServer.pingpongInfo).db_size;
	return(DBServer.pingpongInfo).db_pp_as + index * DBServer.unitSize;
}

int pingpong_write(int index, void* value)
{
	if (index > (DBServer.pingpongInfo).db_size)
		index = index % (DBServer.pingpongInfo).db_size;

	memcpy((DBServer.pingpongInfo).db_pp_as + index * DBServer.unitSize, value, DBServer.unitSize);
    db_lock( &(DBServer.pingpongInfo.db_pp_lock));
	if (0 == (DBServer.pingpongInfo).current) {

		memcpy((DBServer.pingpongInfo).db_pp_as_odd + index * DBServer.unitSize + index % DBServer.unitSize, value, 4);
		(DBServer.pingpongInfo).db_pp_odd_ba[index] = 1;
	} else {

		memcpy((DBServer.pingpongInfo).db_pp_as_even + index * DBServer.unitSize + index % DBServer.unitSize, value, 4);
		(DBServer.pingpongInfo).db_pp_even_ba[index] = 1;
	}
    db_unlock( &(DBServer.pingpongInfo.db_pp_lock));
	return 0;
}

void db_pingpong_ckp(int ckp_order, void *pp_info)
{
	int ckp_fd;
	char ckp_name[32];
	int i;
	int db_size;
	db_pingpong_infomation *info;
	char *currentBackup;
	unsigned char *currentBA;
	long long timeStart;
	long long timeEnd;
	
	info = pp_info;
	sprintf(ckp_name, "./ckp_backup/pp_%d", ckp_order);
	if (-1 == (ckp_fd = open(ckp_name, O_WRONLY | O_CREAT, 666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	
	timeStart = get_ntime();
    db_lock( &(DBServer.pingpongInfo.db_pp_lock));
	//prepare for checkpoint
	info->current = !(info->current);
    db_unlock( &(DBServer.pingpongInfo.db_pp_lock));
	if (0 == info->current) {
		currentBackup = info->db_pp_as_odd;
		currentBA = info->db_pp_odd_ba;
	} else {
		currentBackup = info->db_pp_as_even;
		currentBA = info->db_pp_even_ba;
	}
	for (i = 0; i < db_size; i++) {
		if (1 == currentBA[i]) {
			//info->db_pp_as_previous[i] = info->db_pp_as_even[i];
			memcpy(info->db_pp_as_previous + i * DBServer.unitSize,
				currentBackup + i * DBServer.unitSize, DBServer.unitSize);
			memset(currentBackup + i * DBServer.unitSize, 0, DBServer.unitSize);
			currentBA[i] = 0;
		}
	}
	timeEnd = get_ntime();
	add_prepare_log(&DBServer,timeEnd - timeStart);
	timeStart = get_ntime();
	//write to disk
	write(ckp_fd, info->db_pp_as_previous, DBServer.unitSize * db_size);
	fsync(ckp_fd);
	close(ckp_fd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
}

void db_pingpong_destroy(void *pp_info)
{
	db_pingpong_infomation *info;

	info = pp_info;
	free(info->db_pp_as);
	free(info->db_pp_as_even);
	free(info->db_pp_as_odd);
	free(info->db_pp_as_previous);
	free(info->db_pp_even_ba);
	free(info->db_pp_odd_ba);

}
