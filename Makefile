CC=gcc
MPICC=mpic++
RM=rm -rf
BUILD_DIR=build
SRC=./src
FLAGS=-g -O0 -Wno-pointer-arith -fopenmp
PARALLEL_FLAGS=-DPARALLEL=1
MAX_ITERS=5

all: main

build_dir:
	mkdir -p build

matrix_seq: $(SRC)/matrix.c $(SRC)/matrix.h
	$(CC) $(FLAGS) -I$(SRC) $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_seq.o

matrix_par: $(SRC)/matrix.c $(SRC)/matrix.h
	$(CC) $(PARALLEL_FLAGS) $(FLAGS) -I$(SRC) $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_par.o

mpi_slave:
	$(MPICC) $(FLAGS) -I$(SRC) -DMPI_BUILD=1 $(SRC)/mpi_slave.c -c -o $(BUILD_DIR)/mpi_slave.o

matrix_mpi_seq: $(SRC)/matrix.c $(SRC)/matrix.h mpi_slave
	$(MPICC) $(FLAGS) -I$(SRC) -DMPI_BUILD=1 $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_mpi_seq.o

matrix_mpi_par: $(SRC)/matrix.c $(SRC)/matrix.h mpi_slave
	$(MPICC) $(PARALLEL_FLAGS) $(FLAGS) -I$(SRC)  -DMPI_BUILD=1 $(SRC)/matrix.c -c -o $(BUILD_DIR)/matrix_mpi_par.o

seq: $(SRC)/main.c build_dir matrix_seq
	$(CC) $(FLAGS) -I$(SRC) $(SRC)/main.c $(BUILD_DIR)/matrix_seq.o -o $(BUILD_DIR)/experiment_seq

parallel: $(SRC)/main.c build_dir matrix_par
	$(CC) $(PARALLEL_FLAGS) $(FLAGS) -I$(SRC) $(SRC)/main.c $(BUILD_DIR)/matrix_par.o -o $(BUILD_DIR)/experiment_par

mpi_seq: $(SRC)/main.c build_dir matrix_mpi_seq mpi_slave
	$(MPICC) $(FLAGS) -I$(SRC) -DMPI_BUILD=1 $(BUILD_DIR)/mpi_slave.o $(SRC)/main.c $(BUILD_DIR)/matrix_mpi_seq.o -o $(BUILD_DIR)/experiment_mpi_seq_exec
	echo 'mpiexec -n $(MAX_ITERS) $$(dirname $$0)/experiment_mpi_seq_exec $$@' > $(BUILD_DIR)/experiment_mpi_seq
	chmod +x $(BUILD_DIR)/experiment_mpi_seq

mpi_parallel: $(SRC)/main.c build_dir matrix_mpi_par mpi_slave
	$(MPICC) $(PARALLEL_FLAGS) $(FLAGS) -DMPI_BUILD=1 -I$(SRC) $(BUILD_DIR)/mpi_slave.o $(SRC)/main.c $(BUILD_DIR)/matrix_mpi_par.o -o $(BUILD_DIR)/experiment_mpi_par_exec
	echo 'mpiexec -n $(MAX_ITERS) $$(dirname $$0)/experiment_mpi_par_exec $$@' > $(BUILD_DIR)/experiment_mpi_par
	chmod +x $(BUILD_DIR)/experiment_mpi_par

main: seq parallel mpi_seq mpi_parallel

clean:
	$(RM) $(BUILD_DIR)