#!/bin/bash
declare -a undir_graph=(
  # Social
  # "stackoverflow_sym"
 "com-orkut_sym"
#  "soc-LiveJournal1_sym"
#  "twitter_sym"
#  "friendster_sym"
#  "as-skitter_sym"
#  "com-youtube_sym"
#  "wiki-topcats_sym"
#  "enwiki_sym"
#  "graph500-scale25_sym"
# #
# #  # # Web
#  "sd_arc_sym"
# #
# #
# #  # # Road
# #
#  "RoadUSA_sym"
#  "Germany_sym"

#   # # k-NN
#   "Household_lines_5_sym"
#   "CHEM_5_sym"
#  "GeoLifeNoScale_2_sym"
#  "GeoLifeNoScale_5_sym"
#  "GeoLifeNoScale_10_sym"
#  "GeoLifeNoScale_15_sym"
#  "GeoLifeNoScale_20_sym"
#  "Cosmo50_5_sym"

#  # # Synthetic
#  "grid_1000_10000_03_sym"
#  "grid_1000_10000_sym"
#  "grid_4000_4000_03_sym"
#  "grid_4000_4000_sym"

)

declare numactl="numactl -i all"

declare num_batches=10
declare batch_size=100000
declare num_queries=1000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/bench_batch_size
cd ${source_dir}/build/benchmark/


#echo $(pwd)
rm bench_batch_size
make bench_batch_size

for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}.bin
  ${numactl} ./bench_batch_size -a 0 -b ${num_batches} -s ${batch_size} -q ${num_queries} ${data_path}/${graph}.bin ${data_path}/bench_batch_size/${graph}.query
  echo
done

