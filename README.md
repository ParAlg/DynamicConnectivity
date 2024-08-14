# BatchDynamicConnectivity

## How to Run

To build
```
git clone https://github.com/ParAlg/BatchDynamicConnectivity.git
mkdir -p build
cd build
cmake .. -DDOWNLOAD_PARLAY=TRUE -DDOWNLOAD_PARLAY=true -DDOWNLOAD_PAM=true
make -j 192
```
To run experiments, in repo root directory
```
mkdir -p data
ln -s /ssd1/zhongqi/graphdata/sym/* data
cd scripts
./bench_dynamic.sh
```