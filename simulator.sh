DB_SIZE=10485760
UPDATE_FREQUENCY=100
THREAD_NUM=1
echo "NAIVE:"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 0 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/naive_ckp_log NAIVE & 1>/dev/null 2>/dev/null
python ./update_log_plot.py ../log/naive_update_log_0 NAIVE_UPDATE_$UPDATE_FREQUENCY & 1>/dev/null 2>/dev/null 
cd ../
echo "------------------------------------"

echo "COPY ON UPDATE"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 1 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/cou_ckp_log COPY_ON_UPDATE & 1>/dev/null 2>/dev/null
python ./update_log_plot.py ../log/cou_update_log_0 COU_UPDATE_$UPDATE_FREQUENCY & 1>/dev/null 2>/dev/null 
cd ../
echo "------------------------------------"

echo "WAIT-FREE PINGPONG"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 3 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/pingpong_ckp_log PINGPONG & 1>/dev/null 2>/dev/null
python ./update_log_plot.py ../log/pp_update_log_0 PINGPONG_UPDATE_$UPDATE_FREQUENCY & 1>/dev/null 2>/dev/null 
cd ../
echo "------------------------------------"

echo "WAIT-FREE ZIGZAG"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 2 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/zigzag_ckp_log ZIGZAG & 1>/dev/null 2>/dev/null
python ./update_log_plot.py ../log/zigzag_update_log_0 zigzag_UPDATE_$UPDATE_FREQUENCY & 1>/dev/null 2>/dev/null 
cd ../
echo "------------------------------------"
