function calculate_avg_and_dev() {
    ((avg = 0))
    ((dev = 0))
    ((iter = $1))
    ((iters = $1))

    while [[ ${iter} -gt 0 ]] ; do
        res=$(./build/experiment_$2 -t -d $dims)

        avg=$(echo "scale=6; $avg + $res" | bc | sed 's/^\./0./')
        dev=$(echo "scale=6; $dev + $res * $res" | bc | sed 's/^\./0./')
        ((iter -= 1))
    done

    avg=$(echo "scale=6; $avg / $iters" | bc | sed 's/^\./0./')
    dev=$(echo "scale=6; $dev / $iters - $avg * $avg + 0.000001" | bc | sed 's/^\./0./')
    echo "$avg,$dev"
}

make -s
((dims = 128))
((diff = 64))
((iter = 15))
type=$1

echo "Dims,Avg,Dev"
while [[ ${iter} -gt 0 ]] ; do
    res=$(calculate_avg_and_dev 10 $type)
    echo "$dims,$res"

    ((dims += diff))
    ((iter -= 1))
done
