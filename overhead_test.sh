THREAD_NUM='1'
ALG_ARRAY=(0 1 2 3 4 5)
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK")
UNIT_SIZE=8192
DB_SIZE_ARRAY=("256" "512" "768" "1024" "1280" "1536" "1792" "2048" "2304" "2560")
UF_ARRAY=("32" "64" "96" "128" "160" "192" "224" "256" "288" "320")
UF_BASE="32"

RF_FILE="./rfg.txt"
RESULT_DIR="./data/"
LOG_DIR="../log/overhead/"
EXP_DIR="./experimental_result/"
mkdir log
mkdir ckp_backup
mkdir log/overhead
mkdir log/latency

#rm ./log/overhead/*.log
#rm ./log/latency/*.log
for eachSize in 0 1 2 3 4 5 6 7 8 9
do
#	python ./src/zipf_create/uniform_create.py ${RF_FILE} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
	for eachAlg in 0 1 2 3 4 5
	do
	echo ${ALG_NAME[eachAlg]} ${UF_ARRAY[eachSize]} ${DB_SIZE_ARRAY[eachSize]}
	echo "---------------------------------------------"
	ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE_ARRAY[eachSize]}" "$eachAlg" "$RF_FILE" "${UF_ARRAY[eachSize]}" "$UNIT_SIZE
#	./bin/ckp_simulator $ARG_CKP_SIMULATOR
	echo "---------------------------------------------"
	done
done

cd diagrams
for i in 0 1 2 3 4 5
do
	echo $i
		
	python overhead_format.py ${ALG_ARRAY[i]} ${DB_SIZE_ARRAY[0]} $UF_BASE $UNIT_SIZE $RESULT_DIR $LOG_DIR
done
python overhead_savePDF.py $RESULT_DIR $EXP_DIR
cd ../

