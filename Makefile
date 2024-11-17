CC=gcc
RM=rm -rf
BUILD_DIR=build
SRC=./src
FLAGS=-O0 -fopenmp
PARALLEL_FLAGS=-DPARALLEL=1

all: main

build_dir:
	mkdir -p build

matrix_seq: $(SRC)/matrix.c $(SRC)/matrix.h
	$(CC) $(FLAGS) -I$(SRC) $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_seq.o

matrix_par: $(SRC)/matrix.c $(SRC)/matrix.h
	$(CC) $(PARALLEL_FLAGS) $(FLAGS) -I$(SRC) $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_par.o

seq: $(SRC)/main.c build_dir matrix_seq
	$(CC) $(FLAGS) -I$(SRC) $(SRC)/main.c $(BUILD_DIR)/matrix_seq.o -o $(BUILD_DIR)/experiment_seq

parallel: $(SRC)/main.c build_dir matrix_par
	$(CC) $(PARALLEL_FLAGS) $(FLAGS) -I$(SRC) $(SRC)/main.c $(BUILD_DIR)/matrix_par.o -o $(BUILD_DIR)/experiment_par

main: seq parallel

clean:
	$(RM) $(BUILD_DIR)