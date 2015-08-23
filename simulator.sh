DB_SIZE=10485760
echo "NAIVE:"
echo "------------------------------------"
./bin/ckp_simulator 1 $DB_SIZE 0 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/naive_ckp_log NAIVE &
cd ../
echo "------------------------------------"

echo "COPY ON UPDATE"
echo "------------------------------------"
./bin/ckp_simulator 1 $DB_SIZE 1 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/cou_ckp_log COPY_ON_UPDATE &
cd ../
echo "------------------------------------"

echo "WAIT-FREE ZIGZAG"
echo "------------------------------------"
./bin/ckp_simulator 1 $DB_SIZE 2 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/zigzag_ckp_log ZIGZAG &
cd ../
echo "------------------------------------"

echo "WAIT-FREE PINGPONG"
echo "------------------------------------"
./bin/ckp_simulator 1 $DB_SIZE 3 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/pingpong_ckp_log PINGPONG &
cd ../
echo "------------------------------------"
