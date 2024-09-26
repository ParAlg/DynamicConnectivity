#!/bin/bash
declare rmat_id=(16 18 20 22 24 26)

declare num_batches=10

declare num_queries=1000000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/bench_rmat
cd ${source_dir}/build/benchmark/

declare target="bench_rmat"

exp=$1
if [ $exp == "memory" ]; then
  target="bench_ram_rmat"
fi
if [ $exp == "time" ]; then
  target="bench_rmat"
fi

alg=$2
if [[ ! "$alg" =~ ^[0-4]$ ]]; then
  alg=0
fi

echo ${alg}

#echo $(pwd)
rm ${target}
make -j ${target}

for graph in "${rmat_id[@]}"; do
  echo Running on rmat_${graph}
  ./${target} -a ${alg} -b ${num_batches} -q ${num_queries} -r ${graph}
  echo
done

