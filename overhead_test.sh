THREAD_NUM='1'
ALG_ARRAY=(0 1 2 3 4)
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK")
UNIT_SIZE=8192
DB_SIZE_ARRAY=("25600" "51200" "76800" "102400" "128000" "153600" "179200" "204800" "230400" "256000")
UF_ARRAY=("32" "64" "96" "128" "160" "192" "224" "256" "288" "320")
UF_BASE=32000
RF_FILE="./rfg.txt"
RESULT_DIR="./data/"
LOG_DIR="../log/overhead/"
for j in 0
do
	python ./src/zipf_create/uniform_create.py ${RF_FILE} ${UF_ARRAY[j]} ${DB_SIZE_ARRAY[j]}
	for i in 0 2 
	do
	echo ${ALG_NAME[i]} ${UF_ARRAY[j]} ${DB_SIZE_ARRAY[j]}
	ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE_ARRAY[j]}" "$i" "$RF_FILE" "${UF_ARRAY[j]}" "$UNIT_SIZE
	./bin/ckp_simulator $ARG_CKP_SIMULATOR
	done
done
cd diagrams
for i in 0 1 2 3 4
do
	
	python overhead_format.py ${ALG_ARRAY[i]} ${DB_SIZE_ARRAY[0]} $UF_BASE $UNIT_SIZE $RESULT_DIR $LOG_DIR
done
cd ../
