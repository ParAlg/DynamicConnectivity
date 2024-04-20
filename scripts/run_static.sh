#!/bin/bash
declare -a undir_graph=(
  # Social

#  "com-orkut_sym"
#  "soc-LiveJournal1_sym"
#  "twitter_sym"
#  "friendster_sym"
#
#  # # Web
#  "sd_arc_sym"
#
#
#  # # Road
#
#  "RoadUSA_sym"
#  "Germany_sym"

  # # k-NN
  "Household.lines_5_sym"
#  "CHEM_5_sym"
#  "GeoLifeNoScale_2_sym"
#  "GeoLifeNoScale_5_sym"
#  "GeoLifeNoScale_10_sym"
#  "GeoLifeNoScale_15_sym"
#  "GeoLifeNoScale_20_sym"
#  "Cosmo50_5_sym"
#
#  # # Synthetic
#  "grid_1000_10000_03_sym"
#  "grid_1000_10000_sym"
#  "grid_4000_4000_03_sym"
#  "grid_4000_4000_sym"

)

declare numactl="numactl -i all"

declare num_batches=10

declare num_queries=1000

declare source_dir="$(dirname $(pwd))"
declare data_path="${source_dir}/data"
# echo $data_path
# echo $source_dir

mkdir ${data_path}/static_connectivity
cd ${source_dir}/build/benchmark/static_connectivity/


#echo $(pwd)
rm static_connectivity
make static_connectivity

for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}.bin
  ${numactl} ./static_connectivity -b ${num_batches} -q ${num_queries} ${data_path}/${graph}.bin ${data_path}/static_connectivity/${graph}.query
  echo
done

