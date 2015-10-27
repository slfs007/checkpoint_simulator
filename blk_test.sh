THREAD_NUM='1'
ALG_ARRAY=(0 1 2 3 4 5)
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK" "LL")
UNIT_SIZE=("8192" "4096" "2048" "1024" "512")
DB_SIZE_ARRAY=("25600" "51200" "102400" "204800" "409600")
UF_ARRAY=("32" "64" "128" "256" "512")
UF_BASE=32000
RF_FILE="./rfg.txt"
RESULT_DIR="./expiremental/"
DATA_DIR="../log/overhead/"
rm ./log/overhead/*.log
rm ./log/latency/*.log

for eachSize in 0 1 2 3 4
do
	python ./src/zipf_create/uniform_create.py ${RF_FILE} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
	for eachAlg in $ALG_ARRAY 
	do
		echo ${ALG_NAME[eachAlg]} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
		echo "---------------------------------------------"
		ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE_ARRAY[eachSize]}" "$eachAlg" "$RF_FILE" "${UF_ARRAY[eachSize]}" "${UNIT_SIZE[eachSize]}
		./bin/ckp_simulator $ARG_CKP_SIMULATOR
		echo "---------------------------------------------"
	done
done

cd diagrams
BLK_ARG=${DB_SIZE_ARRAY[0]}" "${UF_ARRAY[0]}" "${UNIT_SIZE[0]}" "${RESULT_DIR}" "${DATA_DIR}
python blkSizeOverhead.py $BLK_ARG
cd ../
