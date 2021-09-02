
```
mkdir ~/.local/tbb/
mkdir ~/.local/tbb/lib
ln -s ~/.local/tbb/lib/ ~/.local/tbb/lib64

cd ~/.local/src/
wget https://gitee.com/rrzhang/oneTBB/repository/archive/v2020.3.zip
unzip v2020.3.zip > /dev/null
cd oneTBB
make -j32
cd build
chmod +x *.sh
sh generate_tbbvars.sh
sh tbbvars.sh
cd linux_intel64_gcc_cc7.4.0_libc2.17_kernel5.0.5_release

cd linux_intel64_gcc_cc7.4.0_libc2.17_kernel5.0.5_release

cp *.so ~/.local/tbb/lib/
cp *.so.2 ~/.local/tbb/lib/
cp -r ~/.local/src/oneTBB/include/ ~/.local/tbb/

```

```
# .bashrc
# tbb
export           TBB_ROOT=$HOME/.local/tbb
export    LD_LIBRARY_PATH=$TBB_ROOT/lib:$LD_LIBRARY_PATH
export       LIBRARY_PATH=$LIBRARY_PATH:$LD_LIBRARY_PATH
export        LD_RUN_PATH=$LD_RUN_PATH:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$TBB_ROOT/include
export     C_INCLUDE_PATH=$C_INCLUDE_PATH:$CPLUS_INCLUDE_PATH
```

v2021.3.0 版本：
~~~cpp
wget https://gitee.com/rrzhang/hwloc/repository/archive/hwloc-2.4.0.zip
unzip hwloc-2.4.0.zip > /dev/null
cd hwloc
./autogen.sh
./configure --prefix=/home/zhangrongrong/.local/hwloc-2.4.0
make -j32 && make install
cd ..
rm -r hwloc/


cd ~/.local/src/
wget https://gitee.com/rrzhang/oneTBB/repository/archive/v2021.3.0.zip
mv v2021.3.0.zip oneTBB-2021.3.0.zip
unzip oneTBB-2021.3.0.zip > /dev/null
cd oneTBB
mkdir build && cd build
cmake .. -DCMAKE_HWLOC_2_4_LIBRARY_PATH="/home/zhangrongrong/.local/hwloc-2.4.0/lib/libhwloc.so" -DCMAKE_HWLOC_2_4_INCLUDE_PATH="/home/zhangrongrong/.local/hwloc-2.4.0/include" -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/oneTBB-2021.3.0
make -j32 && make install
~~~