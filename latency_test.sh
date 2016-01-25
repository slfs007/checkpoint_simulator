DB_SIZE='256'
UF='32'
THREAD_NUM='1'
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK" "LL")
LOG_NAME=("naive" "cou" "zigzag" "pingpong" "mk" "ll")
UNIT_SIZE=4096
RF_FILE="./rfg.txt"
DATA_DIR="./log/latency/"
RESULT_DIR="./diagrams/experimental_result/"
mkdir log
mkdir ckp_backup

#1. generate the zipf random file
python ./src/zipf_create/Zipf.py $RF_FILE $UF $DB_SIZE
for i in 0 1 2 3 4 5 
do 
	echo ${ALG_NAME[i]}
	echo "-------------------------------------"
	ARG_CKP_LATENCY=${THREAD_NUM}" "${DB_SIZE}" "$i" "$RF_FILE" "${UF}" "$UNIT_SIZE
	./bin/ckp_simulator $ARG_CKP_LATENCY 
	echo "-------------------------------------"
done


PLOT_ARG=$UNIT_SIZE" "$DB_SIZE" "$UF" 0 "$DATA_DIR" "$RESULT_DIR
python ./diagrams/latency_test.py $PLOT_ARG


