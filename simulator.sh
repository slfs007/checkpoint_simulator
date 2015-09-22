DB_SIZE='25600'
UPDATE_FREQUENCY='320'
THREAD_NUM='1'
ALG_NAME=("NAIVE" "COU" "ZIGZAG" "PINGPONG" "MK")
LOG_NAME=("naive" "cou" "zigzag" "pingpong" "mk")
AVG_DIVISOR=1000
<<<<<<< HEAD
UNIT_SIZE=8192
DB_SIZE_ARRAY=("25600" "51200" "76800" "102400" "128000" "153600" "179200" "204800" "230400" "256000")
UF_ARRAY=("32" "64" "96" "128" "160" "192" "224" "256" "288" "320")
=======
UNIT_SIZE=4096
mkdir log
mkdir ckp_bakcup
>>>>>>> 58e985161e81b5bcc9ed800885d17e07c42a8473
for i in 0 1 2 3 4  
do
	for j in 0 1 2 3 4 5 6 7 8 9
	do
	echo ${ALG_NAME[i]}
	echo "------------------------------------"
	ARG_CKP_SIMULATOR=${THREAD_NUM}" "${DB_SIZE_ARRAY[j]}" "$i" ./rfg.txt "${UF_ARRAY[j]}" "$UNIT_SIZE
	echo $ARG_CKP_SIMULATOR
	./bin/ckp_simulator $ARG_CKP_SIMULATOR
	echo "------------------------------------"
	done
done

for i in 0 1 2 3 4
do
	ARG_PLOT_MAIN="../log/"${LOG_NAME[i]}"_ckp_log "${ALG_NAME[i]}
	ARG_PLOT_UPDATE="../log/"${LOG_NAME[i]}"_update_log_0 "${ALG_NAME[i]}"_"${UPDATE_FREQUENCY}" "$AVG_DIVISOR
	ARG_FOR_GNUPLOT="../log/"${LOG_NAME[i]}"_update_log_0 "${ALG_NAME[i]}"_"${UPDATE_FREQUENCY}"_"
	cd plot_practice
#	python ./plot_main.py $ARG_PLOT_MAIN & 
#	python ./update_log_plot.py $ARG_PLOT_UPDATE &
#	python ./prepare_for_gnuplot.py $ARG_FOR_GNUPLOT
	cd ../
done

cd plot_practice
#gnuplot gnuplot_run.sh
cd ../
