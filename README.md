# BatchDynamicConnectivity

## How to Run

To build
```
git clone https://github.com/ParAlg/BatchDynamicConnectivity.git
mkdir -p build
cd build
cmake .. -DDOWNLOAD_PARLAY=TRUE -DDOWNLOAD_PARLAY=true -DDOWNLOAD_PAM=true -DCMAKE_BUILD_TYPE=release
make -j 192
```
To run experiments, in repo root directory
```
mkdir -p data
ln -s /ssd1/zhongqi/graphdata/sym/*.bin data
cd scripts
#To run time experiment
./bench_dynamic.sh time
#Tun run memory experiment
./bench_dynamic.sh memory

# in bench_dynamic.sh
# 0 all algorithms
# 1 sequtial hdt
# 2 insert to ROOT
# 3 insert to LCA
```