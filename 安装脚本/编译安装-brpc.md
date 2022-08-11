# 编译安装 brpc
## 准备
### 依赖

| 包名 | 版本 |
| --------- | --------- |
| brpc | 62240fea920006dddfc2cb58b98a15e323ed499f 分支 |
| gflags | 2.2.2 |
| protobuf | 4.0.x |
| leveldb | 1.23 |
| openssl | 1.0.2k |
| boost | - |

brpc 依赖 protobuf，当修改 CMakeList.txt，使链接指向自己目录下 protobuf 时，编译中途仍然链接的是 /usr 下的库，几番折腾，未果。

遂尝试自己安装这些库，并为 cmake 指定目录。所有的库都安装在 `brpc-env` 目录下，具体 :  

`brpc-rdma/	`、
  
`brpc-1.2.0/`、  

`brpc-env/gflags-2.2.2`、  

`brpc-env/leveldb-1.23`、  

`brpc-env/openssl-1.0.2k`、  

`brpc-env/protobuf-4.0.x` 

## 注意

#### 1
编译RDMA版本时，需要修改 `src/brpc/rdma/rdma_endpoint.cpp:50` 中的变量 `rdma_disable_local_connection` 为false，这样 RDMA 服务端才能在ip `0.0.0.0`或`127.0.0.1` 上启动。
#### 2
LOG(FATAL) 默认并不能使程序崩溃，需要修改 `src/butil/logging.cc:118` 中变量 `crash_on_fatal_log` 为 true 才行。

#### 3
brpc 默认使用 bthread，使用协程实现，在使用时服务端的回调函数逻辑里，我们使用了大量的锁，结果导致 bthread 里很多逻辑也阻塞住了，结果就是系统陷入了死锁，gdb 查看的时候，全部是等待锁。  
编译的时候直接让一个 bthread 对应一个 thread，编辑 `src/brpc/event_dispatcher.cpp` 把变量 `usercode_in_pthread` true。  
参考[brpc 服务程序假死
#412](https://github.com/apache/incubator-brpc/issues/412) 中 gtarcoder commented on 24 Oct 2018

## 安装 brpc
~~~
// 确保 protobuf/bin/protoc export 于 ~/.bashrc，然后 source ~/.bashrc

cd $HOME/.local/src/incubator-brpc
git clone https://gitee.com/baidu/BRPC.git 
mv BRPC incubator-brpc
cd incubator-brpc
git remote add github2 git@github.com:apache/incubator-brpc.git
git pull github2 master:master
~~~


### cmake 安装：
在 CmakeList.txt 中加入：`cmake_policy(SET CMP0074 NEW)`。
~~~
git checkout rdma
git reset --hard
git clean -d -fx

mkdir build && cd build

// 这是指定 rdma 头文件目录的，可以不执行
unset CPLUS_INCLUDE_PATH C_INCLUDE_PATH LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$HOME/.local/include
export C_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$HOME/.local/include

cmake -DWITH_RDMA=on \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/brpc-rdma \
-DCMAKE_INCLUDE_PATH="/home/zhangrongrong/.local/boost_1_73_0;/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/include;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/include;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/include;/home/zhangrongrong/.local/openssl-1.0.2k/include" \
-DCMAKE_LIBRARY_PATH="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64;/home/zhangrongrong/.local/openssl-1.0.2k/lib" \
..

or

cmake -DWITH_RDMA=on \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/brpc-rdma \
-DCMAKE_PREFIX_PATH="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2;/home/zhangrongrong/.local/brpc-env/leveldb-1.23;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x;home/zhangrongrong/.local/brpc-env/protobuf-4.0.x" \
..

cmake --build . -j32 && make install
cp -r output/bin/ ~/.local/brpc-rdma/

cd ~/.local/
ln -s boost_1_73_0/ boost_1_73_0-for-brpc-with-rdma
~~~

非 rdma，1.2.0 版本，需要先指定依赖库的位置，不然后面链接时报错，1.1.0 之前的版本不需要这个步骤：
`export LD_LIBRARY_PATH=/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib:/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib:/home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64:/home/zhangrongrong/.local/openssl-1.0.2k/lib:$LD_LIBRARY_PATH`
`export LIBRARY_PATH=/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib:/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib:/home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64:/home/zhangrongrong/.local/openssl-1.0.2k/lib:$LIBRARY_PATH`

修改 CMakeList.txt，加入：
`cmake_policy(SET CMP0074 NEW)`

~~~
cd incubator-brpc
git branch release-1.2 29491107
git checkout release-1.2

cmake \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/brpc-1.2.0 -DBUILD_SHARED_LIBS=on \
-DCMAKE_INCLUDE_PATH="/home/zhangrongrong/.local/boost_1_73_0;/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/include;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/include;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/include;/home/zhangrongrong/.local/openssl-1.0.2k/include" \
-DCMAKE_LIBRARY_PATH="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64;/home/zhangrongrong/.local/openssl-1.0.2k/lib" \
..

or

cmake \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/brpc-1.2.0 -DBUILD_SHARED_LIBS=on \
-DCMAKE_PREFIX_PATH="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2;/home/zhangrongrong/.local/brpc-env/leveldb-1.23;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x;home/zhangrongrong/.local/brpc-env/protobuf-4.0.x" \
..

cmake --build . -j32 && make install
cp -r output/bin/ ~/.local/brpc-1.2.0/
~~~


### config 编译
~~~
sh config_brpc.sh  \
--headers="/home/zhangrongrong/.local/boost_1_73_0 /home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/include /home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/include /home/zhangrongrong/.local/brpc-env/leveldb-1.23/include /home/zhangrongrong/.local/openssl-1.0.2k/include /usr/include" \
--libs="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib /home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib /home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64 /home/zhangrongrong/.local/openssl-1.0.2k/lib /usr/lib64 /usr/bin"
or 可使用分号作为分隔符
sh config_brpc.sh  \
--headers="/home/zhangrongrong/.local/boost_1_73_0 /home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/include;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/include;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/include;/home/zhangrongrong/.local/openssl-1.0.2k/include;/usr/include" \
--libs="/home/zhangrongrong/.local/brpc-env/gflags-v2.2.2/lib;/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib;/home/zhangrongrong/.local/brpc-env/leveldb-1.23/lib64;/home/zhangrongrong/.local/openssl-1.0.2k/lib;/usr/lib64;/usr/bin"

make -j32
~~~
> 没有 make install



