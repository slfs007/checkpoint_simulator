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
rm ./log/overhead/*.log
rm ./log/latency/*.log
for eachSize in 0
do
	python ./src/zipf_create/uniform_create.py ${RF_FILE} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
	for eachAlg in 0 
	do
	echo ${ALG_NAME[eachAlg]} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
	echo "---------------------------------------------"
	ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE_ARRAY[eachSize]}" "$eachAlg" "$RF_FILE" "${UF_ARRAY[eachSize]}" "$UNIT_SIZE
	./bin/ckp_simulator $ARG_CKP_SIMULATOR
	echo "---------------------------------------------"
	done
done

cd diagrams
for i in 0 1 2 3 4
do
	echo "i"	
#	python overhead_format.py ${ALG_ARRAY[i]} ${DB_SIZE_ARRAY[0]} $UF_BASE $UNIT_SIZE $RESULT_DIR $LOG_DIR
done
cd ../
