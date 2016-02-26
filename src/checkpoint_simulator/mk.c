#include"mk.h"
extern db_server DBServer;

void db_mk_lock(int index)
{
    unsigned char expected = 0;

    while(!__atomic_compare_exchange_1(DBServer.mkInfo.db_mk_access + index,&expected,
                                1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
        expected = 0;
    }
}
void db_mk_unlock(int index)
{
    __atomic_store_n(DBServer.mkInfo.db_mk_access+index,0,__ATOMIC_SEQ_CST);
}
int db_mk_init(void *mk_info, int db_size)
{
	db_mk_infomation *info = mk_info;

	info->db_size = db_size;

	if (NULL == (info->db_mk_as1 = malloc(DBServer.unitSize * db_size))) {
		perror("db_mk_as1 malloc error");
		return -1;
	}
	memset(info->db_mk_as1, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_mk_as2 = malloc(DBServer.unitSize * db_size))) {
		perror("db_mk_as2 malloc error");
		return -1;
	}
	memset(info->db_mk_as2, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_mk_ba = malloc(db_size))) {
		perror("db_mk_ba malloc error");
		return -1;
	}
	memset(info->db_mk_ba, 0, db_size);

    info->db_mk_access = malloc(db_size);
    memset(info->db_mk_access,0,db_size);
	pthread_rwlock_init(&(info->db_rwlock), NULL);
	info->current = 1;
	return 0;

}

void* mk_read(int index)
{
	if (index > (DBServer.mkInfo).db_size)
		index = index % (DBServer.mkInfo).db_size;
	if (1 == (DBServer.mkInfo).current) {
		return(DBServer.mkInfo).db_mk_as1 + index * DBServer.unitSize;
	} else {
		return(DBServer.mkInfo).db_mk_as2 + index * DBServer.unitSize;
	} 
	return NULL;
}

int mk_write(int index, void* value)
{
	if (index > (DBServer.mkInfo).db_size)
		index = index % (DBServer.mkInfo).db_size;

	pthread_rwlock_rdlock(&((DBServer.mkInfo).db_rwlock));
    db_mk_lock(index);
	if (1 == (DBServer.mkInfo).current) {
		
		memcpy((DBServer.mkInfo).db_mk_as1 + index * DBServer.unitSize , value, DBServer.unitSize);
		(DBServer.mkInfo).db_mk_ba[index] = 1;
	} else {
		
		memcpy((DBServer.mkInfo).db_mk_as2 + index * DBServer.unitSize , value, DBServer.unitSize);
		(DBServer.mkInfo).db_mk_ba[index] = 2;
	}
    db_mk_lock(index);
	pthread_rwlock_unlock(&((DBServer.mkInfo).db_rwlock));
	return 0;
}
typedef struct {
	int fd;
	char *addr;
	int len;
}mk_disk_info;
void *mk_write_to_disk_thr(void *arg)
{
	mk_disk_info *info = arg;
	long long timeStart;
	long long timeEnd;
	timeStart = get_ntime();
	write(info->fd,info->addr,info->len);
	fsync(info->fd);
	close(info->fd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
	return NULL;
}
void db_mk_ckp(int ckp_order, void *mk_info)
{
	int ckp_fd;
	char ckp_name[32];
	int i;
	int db_size;
	db_mk_infomation *info;
	mk_disk_info mkDiskInfo;
	pthread_t mkDiskThrId;
	int mkCur;
	char *backup;
	char *online;
	long long timeStart;
	long long timeEnd;
	info = mk_info;
	sprintf(ckp_name, "./ckp_backup/mk_%d", ckp_order);
	if (-1 == (ckp_fd = open(ckp_name, O_WRONLY | O_CREAT, 666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	
	timeStart = get_ntime();
	pthread_rwlock_wrlock(&(info->db_rwlock));
	if ( info->current == 1)
		info->current = 2;
	else
		info->current = 1;
	//info->current = (1 == (info->current)) ? 2 : 1;
	pthread_rwlock_unlock(&(info->db_rwlock));
    timeEnd = get_ntime();
    add_prepare_log( &DBServer, timeEnd - timeStart);

    timeStart = get_ntime();
    if (1 == info->current) {
		mkCur = 1;
		online = info->db_mk_as1;
		backup = info->db_mk_as2;
		
	} else {
		mkCur = 2;
		online = info->db_mk_as2;
		backup = info->db_mk_as1;
	}
    write(ckp_fd,backup,DBServer.dbSize * DBServer.unitSize);
    fsync(ckp_fd);
    close(ckp_fd);
/*	mkDiskInfo.fd = ckp_fd;
	mkDiskInfo.len = DBServer.dbSize * DBServer.unitSize;
	mkDiskInfo.addr = backup;
	pthread_create(&mkDiskThrId,NULL,mk_write_to_disk_thr,&mkDiskInfo);
    */
	for (i = 0; i < db_size; i++) {
        db_mk_lock(i);
		if (mkCur != info->db_mk_ba[i] && 0 != mkCur) {
			memcpy(online + i * DBServer.unitSize,
				backup + i * DBServer.unitSize, DBServer.unitSize);
			info->db_mk_ba[i] = 0;
		}
        db_mk_unlock(i);
	}
    timeEnd = get_ntime();
    add_overhead_log(&DBServer,timeEnd - timeStart);
}

void db_mk_destroy(void *mk_info)
{
	db_mk_infomation *info = mk_info;
	free(info->db_mk_as1);
	free(info->db_mk_as2);
	free(info->db_mk_ba);
    free(info->db_mk_access);
	pthread_rwlock_destroy(&(info->db_rwlock));
}
