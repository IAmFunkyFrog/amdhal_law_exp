
make
echo "Verification of parallel vs non parallel version"
for seed in `seq 0 1 100`; do
    seq=$(./build/experiment_seq -v -s $seed)
    par=$(./build/experiment_par -v -s $seed)
    if [[ $text == *"$string"* ]]; then
        echo "Test $seed passed"
    else
        echo "Test $seed not passed"
        echo "$seq != $par"
        exit
    fi
done
