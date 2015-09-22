BASE_SIZE=25600
BASE_UF=32000
UNIT_SIZE=8192
ALG_ARRAY=(0 1 2 3 4)
RESULT_DIR="./data/"
LOG_DIR="../plot_practice/overhead/"

for i in 0 1 2 3 4
do
	
	python overhead_format.py ${ALG_ARRAY[i]} $BASE_SIZE $BASE_UF $UNIT_SIZE $RESULT_DIR $LOG_DIR
done
