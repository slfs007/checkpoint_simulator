echo "NAIVE:"
echo "------------------------------------"
./bin/ckp_simulator 1 83886080 0 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/naive_ckp_log NAIVE &
cd ../
echo "------------------------------------"

echo "COPY ON UPDATE"
echo "------------------------------------"
./bin/ckp_simulator 1 83886080 1 ./rfg.txt
cd plot_practice
python ./plot_main.py ../log/cou_ckp_log COPY_ON_UPDATE
cd ../
echo "------------------------------------"
