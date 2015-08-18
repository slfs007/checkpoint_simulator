./bin/ckp_simulator 1 102400 0 ./rfg.txt
cd plot_practice
python plot_main.py 
cd ../
./bin/ckp_simulator 1 102400 1 ./rfg.txt
cd plot_practice
python plot_main.py 
cd ../
