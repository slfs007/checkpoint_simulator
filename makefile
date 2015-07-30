all:rf_generator ckp_simulator
.PHONY:all

rf_generator:
	gcc random_file_generator.c -o rf_generator
ckp_simulator:
	gcc checkpoint_simulator.c -o ckp_simulator
clean:
	rm  rf_generator ckp_simulator