#!/bin/bash
declare -a undir_graph=(
  # Social
 "stackoverflow_sym"
 "com-orkut_sym"
 "soc-LiveJournal1_sym"
 "twitter_sym"
 "friendster_sym"
 "as-skitter_sym"
 "com-youtube_sym"
 "wiki-topcats_sym"
 "enwiki_sym"
 "pokec_sym"

# #  # # Road
# #
#  "RoadUSA_sym"
#  "Germany_sym"

#   # # k-NN
#   "Household_lines_5_sym"
#   "CHEM_5_sym"
)

declare numactl=""

declare num_batches=10

declare num_queries=1000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/bench_dynamic
cd ${source_dir}/build/benchmark/


#echo $(pwd)
rm bench_dynamic
make bench_dynamic

for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}.bin
  ${numactl} ./bench_dynamic -a 0 -b ${num_batches} -q ${num_queries} ${data_path}/${graph}.bin ${data_path}/bench_dynamic/${graph}.query
  echo
done

