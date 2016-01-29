#可更改
DB_SIZE='256'
UF='32'
THREAD_NUM='10'
UNIT_SIZE=4096
#不可更改
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK" "LL")
LOG_NAME=("naive" "cou" "zigzag" "pingpong" "mk" "ll")
RF_FILE="./rfg.txt"
DATA_DIR="../log/overhead/"
RESULT_DIR="./experimental_result/"
mkdir log
mkdir ckp_backup
mkdir log/overhead
mkdir log/latency
#1. generate the zipf random file
python ./src/zipf_create/Zipf.py $RF_FILE $UF $DB_SIZE

for i in 0 1 2 3 4 5 
do 
	echo ${ALG_NAME[i]}
	echo "-------------------------------------"
	ARG_CKP_LATENCY=${THREAD_NUM}" "${DB_SIZE}" "$i" "$RF_FILE" "${UF}" "$UNIT_SIZE
#	./bin/ckp_simulator $ARG_CKP_LATENCY 
	echo "-------------------------------------"
done

cd diagrams
PLOT_ARG=${DATA_DIR}" "${DB_SIZE}" "${UNIT_SIZE}" "${THREAD_NUM}" "${RESULT_DIR}
python tps_plot.py $PLOT_ARG
cd ../
