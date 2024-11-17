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
((dims = 832))
p=1
iter=$(nproc --all)
type=$1
avg_for_seq=$(calculate_avg 10 seq 1)

echo "Proc,AmdhalLimit,Calculated"
while [[ ${iter} -gt 0 ]] ; do
    avg_for_par=$(calculate_avg 10 par $p)
    speedup=$(echo "scale=6; $avg_for_seq / $avg_for_par" | bc | sed 's/^\./0./')
    echo "$p,$p,$speedup"

    ((dims += diff))
    ((iter -= 1))
    ((p += 1))
done
