BASE_SIZE=25600
BASE_UF=32
UNIT_SIZE=8192
ALG_ARRAY=(0 1 2 3 4)
RESULT_DIR="./data/"
LOG_DIR="../log/overhead/"
EXP_DIR="./experimental_result/"
for i in 0 1 2 3 4
do
	
	python overhead_format.py ${ALG_ARRAY[i]} $BASE_SIZE $BASE_UF $UNIT_SIZE $RESULT_DIR $LOG_DIR
done
python overhead_savePDF.py $RESULT_DIR $EXP_DIR
