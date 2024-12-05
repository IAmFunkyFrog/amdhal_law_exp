mkdir -p results

bash ./run_experiment.sh mpi_par > ./results/mpi_par.csv
bash ./run_experiment.sh mpi_seq > ./results/mpi_seq.csv
bash ./run_experiment.sh par > ./results/par.csv
bash ./run_experiment.sh seq > ./results/seq.csv
bash ./compare_with_amdhal.sh > ./results/amdhal_limit.csv