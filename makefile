all:rf_generator ckp_simulator
.PHONY:all

rf_generator:
	gcc ./src/random_file_generator/random_file_generator.c -o ./bin/rf_generator -Wall
ckp_simulator:
	gcc ./src/checkpoint_simulator/checkpoint_simulator.c ./src/checkpoint_simulator/database.c ./src/checkpoint_simulator/update.c -o ./bin/ckp_simulator -pthread -Wall
clean:
	rm  ./bin/*