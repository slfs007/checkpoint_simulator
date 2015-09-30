DB_SIZE='25600'
UPDATE_FREQUENCY='320'
THREAD_NUM='1'
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK")
LOG_NAME=("naive" "cou" "zigzag" "pingpong" "mk")
UNIT_SIZE=8192
RF_FILE="./rfg.txt"
mkdir log
mkdir ckp_backup

#1. generate the zipf random file
python ./src/zipf_create/Zipf.py $RF_FILE $UPDATE_FREQUENCY $DB_SIZE
for i in 0 1 2 3 4
do 
	echo ${ALG_NAME[i]}
	echo "-------------------------------------"
	ARG_CKP_LATENCY=${THREAD_NUM}" "${DB_SIZE}" "$i" "$RF_FILE" "${UPDATE_FREQUENCY}" "$UNIT_SIZE
	./bin/ckp_simulator $ARG_CKP_LATENCY
	echo "-------------------------------------"
done