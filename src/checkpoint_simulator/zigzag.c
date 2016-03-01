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
    info->db_zigzag_lock = UNLOCK;
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
    db_lock( &(DBServer.zigzagInfo.db_zigzag_lock));
	if (0 == (DBServer.zigzagInfo).db_zigzag_mw[index]) {
		//(DBServer.zigzagInfo).db_zigzag_as0[index] = value;        
		memcpy((DBServer.zigzagInfo).db_zigzag_as0 + index * DBServer.unitSize , value, DBServer.unitSize);
	} else {
		//(DBServer.zigzagInfo).db_zigzag_as1[index] = value;
		memcpy((DBServer.zigzagInfo).db_zigzag_as1 + index * DBServer.unitSize , value, DBServer.unitSize);
	}
	(DBServer.zigzagInfo).db_zigzag_mr[index] = (DBServer.zigzagInfo).db_zigzag_mw[index];
    db_unlock( &(DBServer.zigzagInfo.db_zigzag_lock));
	return 0;
}

void db_zigzag_ckp(int ckp_order, void *zigzag_info)
{
	int ckpfd;
	char ckp_name[128];
	int i;
	int db_size;
	db_zigzag_infomation *info;
	long long timeStart;
	long long timeEnd;
	
	info = zigzag_info;
	sprintf(ckp_name, "./ckp_backup/zz_%d", ckp_order);
	if (-1 == (ckpfd = open(ckp_name,O_WRONLY | O_CREAT,666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	timeStart = get_ntime();
    db_lock( &(DBServer.zigzagInfo.db_zigzag_lock));
	//prepare for checkpoint
	for (i = 0; i < db_size; i++) {
		info->db_zigzag_mw[i] = !(info->db_zigzag_mr[i]);
	}
    db_unlock( &(DBServer.zigzagInfo.db_zigzag_lock));
	timeEnd = get_ntime();
	add_prepare_log(&DBServer,timeEnd - timeStart);
	//write to disk
	timeStart = get_ntime();
	for (i = 0; i < db_size; i++) {
		if (0 == info->db_zigzag_mw[i]) {
			write(ckpfd,info->db_zigzag_as1 + i * DBServer.unitSize,
				DBServer.unitSize);
		} else {
			write(ckpfd,info->db_zigzag_as0 + i * DBServer.unitSize, 
				DBServer.unitSize);
		}
	}
	fsync(ckpfd);
	close(ckpfd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
}

void db_zigzag_destroy(void *zigzag_info)
{
	db_zigzag_infomation *info;
	
	info = zigzag_info;
	free(info->db_zigzag_as0);
	free(info->db_zigzag_as1);
	free(info->db_zigzag_mr);
	free(info->db_zigzag_mw);

}
