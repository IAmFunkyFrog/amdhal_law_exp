
make
echo "Verification of parallel vs non parallel version"
for seed in `seq 0 1 10`; do
    seq=$(./build/experiment_seq -v -s $seed)
    par=$(./build/experiment_par -v -s $seed)
    if [[ $seq == $par ]]; then
        echo "Test $seed passed: $seq == $par"
    else
        echo "Test $seed not passed"
        echo "$seq != $par"
        exit
    fi
done

echo "Verification of mpi without open mp vs non parallel version"
for seed in `seq 0 1 10`; do
    seq=$(./build/experiment_seq -v -s $seed)
    par=$(./build/experiment_mpi_par -v -s $seed)
    if [[ $seq == $par ]]; then
        echo "Test $seed passed: $seq == $par"
    else
        echo "Test $seed not passed"
        echo "$seq != $par"
        exit
    fi
done

echo "Verification of mpi with open mp vs non parallel version"
for seed in `seq 0 1 10`; do
    seq=$(./build/experiment_seq -v -s $seed)
    par=$(./build/experiment_mpi_seq -v -s $seed)
    if [[ $seq == $par ]]; then
        echo "Test $seed passed: $seq == $par"
    else
        echo "Test $seed not passed"
        echo "$seq != $par"
        exit
    fi
done
