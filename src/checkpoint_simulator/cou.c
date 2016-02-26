#include"cou.h"
extern db_server DBServer;

void db_cou_lock(int index)
{
    unsigned char expected = 0;

    while(!__atomic_compare_exchange_1(DBServer.couInfo.db_cou_access + index,&expected,
                                1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
        expected = 0;
    }
}
void db_cou_unlock(int index)
{
    __atomic_store_n(DBServer.couInfo.db_cou_access+index,0,__ATOMIC_SEQ_CST);
}
int db_cou_init(void *cou_info, int db_size)
{
	db_cou_infomation *info;

	info = cou_info;
	info->db_size = db_size;

	if (NULL == (info->db_cou_primary =
		(char *) malloc(DBServer.unitSize * db_size))) {
		perror("db_cou_primary malloc error");
		return -1;
	}
	memset(info->db_cou_primary, 'S', DBServer.unitSize * db_size);

	if (NULL == (info->db_cou_shandow =
		(char *) malloc(DBServer.unitSize * db_size))) {
		perror("db_cou_shandow malloc error");
		return -1;
	}
	memset(info->db_cou_shandow, 'S', DBServer.unitSize * db_size);

    if (NULL == (info->db_cou_curBA =
		(unsigned char *) malloc(db_size))) {
		perror("db_cou_bitarray malloc error");
		return -1;
	}

    if (NULL == (info->db_cou_chgBA =
        (unsigned char *) malloc(db_size))) {
        perror("db_cou_bitarray malloc error");
        return -1;
    }

    if (NULL == (info->db_cou_preBA =
        (unsigned char *) malloc(db_size))) {
        perror("db_cou_bitarray malloc error");
        return -1;
    }
    memset(info->db_cou_curBA, 0, db_size);
    memset(info->db_cou_preBA, 0, db_size);
    memset(info->db_cou_chgBA, 0, db_size);
    info->db_cou_access = malloc(db_size);
    memset(info->db_cou_access,0,db_size);
	pthread_rwlock_init(&(info->db_mutex), NULL);

	return 0;
}

void* cou_read(int index)
{
	void *result;
	if (index > DBServer.dbSize)
		index = index % DBServer.dbSize;
	result = (DBServer.couInfo).db_cou_primary + index * DBServer.unitSize;
	return result;
}

int cou_write(int index, void *value)
{
	if (index > DBServer.dbSize)
		index = index % DBServer.dbSize;
    pthread_rwlock_rdlock(& (DBServer.couInfo.db_mutex));
    if ( !DBServer.couInfo.db_cou_curBA[index]){
        db_cou_lock(index);
        if ( DBServer.couInfo.db_cou_chgBA[index])
            memcpy(DBServer.couInfo.db_cou_shandow,value,DBServer.unitSize);
        DBServer.couInfo.db_cou_curBA[index] = 1;
        db_cou_unlock(index);
    }
    memcpy(DBServer.couInfo.db_cou_primary,value,DBServer.unitSize);
    pthread_rwlock_unlock( &(DBServer.couInfo.db_mutex));
    return 0;
}

void ckp_cou(int ckp_order, void *cou_info)
{
	int ckp_fd;
	char ckp_name[32];
	int i;
	int db_size;
	db_cou_infomation *info;
	long long timeStart;
	long long timeEnd;
    static int times = 0;
	info = cou_info;
	sprintf(ckp_name, "./ckp_backup/cou_%d", ckp_order);
    if (-1 == (ckp_fd = open(ckp_name, O_WRONLY | O_CREAT, 666))) {
		perror("checkpoint file open error,checkout if the ckp_backup directory is exist");
		return;
	}
	db_size = info->db_size;
	timeStart = get_ntime();
	pthread_rwlock_wrlock(&(info->db_mutex));
    for (i = 0; i < db_size; i++) {
        info->db_cou_chgBA[i] = info->db_cou_curBA[i] | info->db_cou_preBA[i];
        info->db_cou_preBA[i] = info->db_cou_curBA[i];
        info->db_cou_curBA[i] = 1;
	}
	pthread_rwlock_unlock(&(info->db_mutex));
	timeEnd = get_ntime();
	add_prepare_log(&DBServer,timeEnd - timeStart);
	
	timeStart = get_ntime();
    if ( !times){
        write(ckp_fd, info->db_cou_shandow, DBServer.unitSize * db_size);
        times++;
    }else{
        for (i = 0;i < db_size; i ++){
            if (info->db_cou_chgBA[i]){
                db_cou_lock(i);
                if (info->db_cou_curBA[i]){
                    db_cou_unlock(i);
                    write(ckp_fd, info->db_cou_shandow + i * DBServer.unitSize,DBServer.unitSize);
                }else{
                    write(ckp_fd, info->db_cou_primary + i * DBServer.unitSize,DBServer.unitSize);
                    db_cou_unlock(i);
                }
            }
        }
    }

	fsync(ckp_fd);
	close(ckp_fd);
	timeEnd = get_ntime();
	add_overhead_log(&DBServer,timeEnd - timeStart);
}

void db_cou_destroy(void *cou_info)
{
	db_cou_infomation *info;

	info = cou_info;

	pthread_rwlock_destroy(&(info->db_mutex));
    free(info->db_cou_chgBA);
    free(info->db_cou_curBA);
    free(info->db_cou_preBA);
	free(info->db_cou_shandow);
	free(info->db_cou_primary);
}
