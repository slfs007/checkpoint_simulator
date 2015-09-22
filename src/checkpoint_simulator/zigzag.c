#include"zigzag.h"
extern db_server DBServer;

int db_zigzag_init(void *zigzag_info, int db_size)
{
	db_zigzag_infomation *info;

	info = zigzag_info;

	info->db_size = db_size;

	if (NULL == (info->db_zigzag_as0 = (char *) malloc(DBServer.unitSize * db_size))) {
		perror("db_zigzag_as0 malloc error");
		return -1;
	}
	memset(info->db_zigzag_as0, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_zigzag_as1 = (char *) malloc(DBServer.unitSize * db_size))) {
		perror("db_zigzag_sa1 malloc error");
		return -1;
	}
	memset(info->db_zigzag_as0, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_zigzag_mr = (unsigned char *) malloc(db_size))) {
		perror("db_zigzag_mr malloc error");
		return -1;
	}
	memset(info->db_zigzag_mr, 0, db_size);

	if (NULL == (info->db_zigzag_mw = (unsigned char *) malloc(db_size))) {
		perror("db_zigzag_mw malloc error");
		return -1;
	}
	memset(info->db_zigzag_mw, 1, db_size);
	pthread_rwlock_init(&(info->write_mutex), NULL);
	return 0;
}

void* zigzag_read(int index)
{
	if (index > (DBServer.zigzagInfo).db_size)
		index = index % (DBServer.zigzagInfo).db_size;
	if (0 == (DBServer.zigzagInfo).db_zigzag_mr[index]) {
		return(void *) ((DBServer.zigzagInfo).db_zigzag_as0 + index * DBServer.unitSize);
	} else {
		return(void *) ((DBServer.zigzagInfo).db_zigzag_as1 + index * DBServer.unitSize);
	}
}

int zigzag_write(int index, void *value)
{
	if (index > (DBServer.zigzagInfo).db_size)
		index = index % (DBServer.zigzagInfo).db_size;
	pthread_rwlock_rdlock(&((DBServer.zigzagInfo).write_mutex));
	if (0 == (DBServer.zigzagInfo).db_zigzag_mw[index]) {
		//(DBServer.zigzagInfo).db_zigzag_as0[index] = value;        
		memcpy((DBServer.zigzagInfo).db_zigzag_as0 + index * DBServer.unitSize + index % DBServer.unitSize, value, 4);
	} else {
		//(DBServer.zigzagInfo).db_zigzag_as1[index] = value;
		memcpy((DBServer.zigzagInfo).db_zigzag_as1 + index * DBServer.unitSize + index % DBServer.unitSize, value, 4);
	}
	(DBServer.zigzagInfo).db_zigzag_mr[index] = (DBServer.zigzagInfo).db_zigzag_mw[index];
	pthread_rwlock_unlock(&((DBServer.zigzagInfo).write_mutex));
	return 0;
}

void db_zigzag_ckp(int ckp_order, void *zigzag_info)
{
	int ckpfd;
	char ckp_name[128];
	int i;
	int db_size;
	db_zigzag_infomation *info;
	int tick = 0;
	info = zigzag_info;
	sprintf(ckp_name, "./ckp_backup/zz_%d", ckp_order);
	if (-1 == (ckpfd = open(ckp_name,O_WRONLY | O_CREAT))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;

	pthread_rwlock_wrlock(&(info->write_mutex));
	clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID * 2]));
	//prepare for checkpoint
	for (i = 0; i < db_size; i++) {
		info->db_zigzag_mw[i] = !(info->db_zigzag_mr[i]);
	}
	pthread_rwlock_unlock(&(info->write_mutex));
	//write to disk
	for (i = 0; i < db_size; i++) {
		if (0 == info->db_zigzag_mw[i]) {
			write(ckpfd,info->db_zigzag_as1 + i * DBServer.unitSize,
				DBServer.unitSize);
			tick++;
		} else {
			write(ckpfd,info->db_zigzag_as0 + i * DBServer.unitSize, 
				DBServer.unitSize);
		}
	}
	printf("tick:%d\n",tick);
	fsync(ckpfd);
	close(ckpfd);
	clock_gettime(CLOCK_MONOTONIC, &(DBServer.ckpTimeLog[DBServer.ckpID * 2 + 1]));
}

void db_zigzag_destroy(void *zigzag_info)
{
	db_zigzag_infomation *info;
	info = zigzag_info;

	free(info->db_zigzag_as0);
	free(info->db_zigzag_as1);
	free(info->db_zigzag_mr);
	free(info->db_zigzag_mw);
	pthread_rwlock_destroy(&(info->write_mutex));
}
