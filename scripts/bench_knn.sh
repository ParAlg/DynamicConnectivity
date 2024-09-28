#!/bin/bash
declare  knn_graph=(10 20 30 40 50 60 70 80 90 100)

declare num_batches=10

declare num_queries=1000000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/bench_knn
cd ${source_dir}/build/benchmark/

declare target="bench_rwg"

# exp=$1
# if [ $exp == "memory" ]; then
#   target="bench_ram_rwg"
# fi
# if [ $exp == "time" ]; then
#   target="bench_rwg"
# fi

alg=$1
if [[ ! "$alg" =~ ^[0-4]$ ]]; then
  alg=0
fi

# echo ${alg}

#echo $(pwd)
rm ${target}
make -j ${target}

for graph in "${knn_graph[@]}"; do
  echo Running on bigann-10M-${graph}.adj
  ./${target} -a ${alg} -b ${num_batches} -q ${num_queries} ${data_path}/bigann-10M-${graph}.adj ${data_path}/bench_knn/${graph}.ans
  echo
done

