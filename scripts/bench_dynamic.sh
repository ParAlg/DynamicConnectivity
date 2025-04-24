#!/bin/bash
declare -a undir_graph=(
  "Germany_sym"
  "RoadUSA_sym"
  "grid_1000_10000_03_sym"
  "Household_lines_5_sym"
  "CHEM_5_sym"
  "twitter_sym"
  "friendster_sym"
  "stackoverflow_sym"
  "com-orkut_sym"
  "soc-LiveJournal1_sym"
  "as-skitter_sym"
  "com-youtube_sym"
  "wiki-topcats_sym"
  "enwiki_sym"
  "pokec_sym"
)

declare num_batches=10

declare num_queries=1000000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/bench_rwg
cd ${source_dir}/build/benchmark/

declare target="bench_rwg"

exp=$1
if [ $exp == "memory" ]; then
  target="bench_ram_rwg"
fi
if [ $exp == "time" ]; then
  target="bench_rwg"
fi

alg=$2
if [[ ! "$alg" =~ ^[0-6]$ ]]; then
  alg=0
fi


#echo $(pwd)
rm ${target}
make ${target}

for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}.bin
  ./${target} -a ${alg} -b ${num_batches} -q ${num_queries} ${data_path}/${graph}.bin ${data_path}/bench_rwg/${graph}.query
  echo
done

