all:rf_generator ckp_simulator
.PHONY:all

rf_generator:
	gcc ./src/random_file_generator/random_file_generator.c -o ./bin/rf_generator -Wall -lrt
ckp_simulator:
	gcc ./src/checkpoint_simulator/checkpoint_simulator.c	\
	    ./src/checkpoint_simulator/naive.c			\
	    ./src/checkpoint_simulator/database.c		\
	    ./src/checkpoint_simulator/update.c			\
	    ./src/checkpoint_simulator/cou.c			\
	    ./src/checkpoint_simulator/zigzag.c			\
	    ./src/checkpoint_simulator/pingpong.c		\
	    ./src/checkpoint_simulator/mk.c			\
	    ./src/checkpoint_simulator/LL.c			\
	    -o ./bin/ckp_simulator -pthread -Wall -lrt -g
clean:
	rm  ./bin/*
