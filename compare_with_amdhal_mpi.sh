function calculate_avg() {
    ((avg = 0))
    ((iter = $1))
    ((iters = $1))

    while [[ ${iter} -gt 0 ]] ; do
        res=$(./build/experiment_$2 -t -d $dims -p $3)

        avg=$(echo "scale=6; $avg + $res" | bc | sed 's/^\./0./')
        ((iter -= 1))
    done

    avg=$(echo "scale=6; $avg / $iters" | bc | sed 's/^\./0./')
    echo $avg
}

make -s
((dims = 2048))
p=1
iter=$(nproc --all)
type=$1
iterations_for_avg=5
avg_for_seq=$(calculate_avg $iterations_for_avg seq 1)

echo "Proc,AmdhalLimit,OpenMP,MPI,OpenMPAndMPI"
while [[ ${iter} -gt 0 ]] ; do
    avg_for_par=$(calculate_avg $iterations_for_avg par $p)
    speedup_omp=$(echo "scale=6; $avg_for_seq / $avg_for_par" | bc | sed 's/^\./0./')
    avg_for_mpi=$(calculate_avg $iterations_for_avg mpi_seq 0)
    speedup_mpi=$(echo "scale=6; $avg_for_seq / $avg_for_mpi" | bc | sed 's/^\./0./')
    avg_for_mpi_omp=$(calculate_avg $iterations_for_avg mpi_par $p)
    speedup_mpi_omp=$(echo "scale=6; $avg_for_seq / $avg_for_mpi_omp" | bc | sed 's/^\./0./')
    echo "$p,$p,$speedup_omp,$speedup_mpi,$speedup_mpi_omp"

    ((dims += diff))
    ((iter -= 1))
    ((p += 1))
done
