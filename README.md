# BatchDynamicConnectivity

## How to Run

To build
```
git clone https://github.com/ParAlg/BatchDynamicConnectivity.git
mkdir -p build
cd build
cmake .. -DDOWNLOAD_PARLAY=TRUE
make -j 192
```
To run experiments, in repo root directory
```
mkdir -p data
ln -s /ssd1/zhongqi/graphdata/sym/* data
cd scripts
#sequential HLT
./run_seq_hdt.sh
#parallel union_find
./run_static.sh
```