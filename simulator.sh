DB_SIZE=52428800
UPDATE_FREQUENCY=100
THREAD_NUM=5
echo "NAIVE:"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 0 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/naive_ckp_log NAIVE & 1>/dev/null 2>/dev/null
cd ../
echo "------------------------------------"

echo "COPY ON UPDATE"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 1 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/cou_ckp_log COPY_ON_UPDATE & 1>/dev/null 2>/dev/null
cd ../
echo "------------------------------------"

echo "WAIT-FREE ZIGZAG"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 2 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/zigzag_ckp_log ZIGZAG & 1>/dev/null 2>/dev/null
cd ../
echo "------------------------------------"

echo "WAIT-FREE PINGPONG"
echo "------------------------------------"
./bin/ckp_simulator $THREAD_NUM $DB_SIZE 3 ./rfg.txt $UPDATE_FREQUENCY
cd plot_practice
python ./plot_main.py ../log/pingpong_ckp_log PINGPONG & 1>/dev/null 2>/dev/null
cd ../
echo "------------------------------------"
