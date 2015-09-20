DB_SIZE='25600'
UPDATE_FREQUENCY='100'
THREAD_NUM='1'
AVG_DIVISOR='1000'
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PIINGPONG" "MK")
LOG_NAME=("naive" "cou" "zigzag" "pingpong" "mk")
AVG_DIVISOR=1000
UNIT_SIZE=4096
for i in 0 1 2 3 4  
do
	echo ${ALG_NAME[i]}
	echo "------------------------------------"
	ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE}" "$i" ./rfg.txt "${UPDATE_FREQUENCY}" "$UNIT_SIZE
	./bin/ckp_simulator $ARG_CKP_SIMULATOR
	echo "------------------------------------"
done

for i in 0 1 2 3 4
do
	ARG_PLOT_MAIN="../log/"${LOG_NAME[i]}"_ckp_log "${ALG_NAME[i]}
	ARG_PLOT_UPDATE="../log/"${LOG_NAME[i]}"_update_log_0 "${ALG_NAME[i]}"_"${UPDATE_FREQUENCY}" "$AVG_DIVISOR
	cd plot_practice
#	python ./plot_main.py $ARG_PLOT_MAIN & 
#	python ./update_log_plot.py $ARG_PLOT_UPDATE &
	cd ../
done
