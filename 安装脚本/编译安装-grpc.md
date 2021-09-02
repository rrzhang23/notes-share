参考 [gRPC C++ - Building from source](https://github.com/grpc/grpc/blob/master/BUILDING.md#pre-requisites)

### 安装gRPC

```
# 安装前清空一些变量
unset CPLUS_INCLUDE_PATH C_INCLUDE_PATH LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH

git clone git@github.com:grpc/grpc.git
git submodule update --init --recursive
```
#### onfigure 模式：
```
cd grpc
git checkout v1.23.x
git reset --hard
git clean -d -f -x
git submodule update --init --recursive

# make
make clean
make -j32
rm -rf $HOME/.local/grpc-v1.23.x
make install prefix=$HOME/.local/grpc-v1.23.x
make clean
```
#### cmake 模式：
grpc > 1.27 且 cmake > 3.13 时可以用 cmake 安装，-DgRPC_XXX_PROVIDER=module 表示通过 third_party 下的源码安装，-DgRPC_XXX_PROVIDER=package 表示通过系统的模块安装。  
1.27 之前，必须使用 package，不然不能 install。
```
cd grpc
git reset --hard
git clean -d -f -x
git submodule update --init --recursive
cd cmake
mkdir build
cd build
# 先安装动态库 .so
cmake -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/grpc             \
-DBUILD_SHARED_LIBS=ON           \
-DCMAKE_BUILD_TYPE=Release       \
-DgRPC_INSTALL=ON                \
-DgRPC_BUILD_TESTS=OFF           \
-DgRPC_ABSL_PROVIDER=module      \
-DgRPC_CARES_PROVIDER=module     \
-DgRPC_PROTOBUF_PROVIDER=module  \
-DgRPC_SSL_PROVIDER=module       \
-DgRPC_ZLIB_PROVIDER=module      \
../..
make -j32 && make install

# 再安装静态库 .so
rm -rf ../build/*
cmake -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/grpc             \
-DCMAKE_BUILD_TYPE=Release       \
-DgRPC_INSTALL=ON                \
-DgRPC_BUILD_TESTS=OFF           \
-DgRPC_ABSL_PROVIDER=module      \
-DgRPC_CARES_PROVIDER=module     \
-DgRPC_PROTOBUF_PROVIDER=module  \
-DgRPC_SSL_PROVIDER=module       \
-DgRPC_ZLIB_PROVIDER=module      \
../..
make -j32 && make install
```
添加软连接
```
cd /home/zhangrongrong/.local/grpc-v1.23.x/lib
ln -s libgrpc++.a libgrpcpp.a
ln -s libgrpc++_error_details.a libgrpcpp_error_details.a
ln -s libgrpc++_reflection.a libgrpcpp_reflection.a
ln -s libgrpc++_unsecure.a libgrpcpp_unsecure.a
```
cmake 模式，参照：[gRPC C++ - Building from source](https://github.com/grpc/grpc/blob/master/BUILDING.md)，不过还是存在问题

##### .bashrc
加入：
```
# gRPC
export          gRPC_ROOT=$HOME/.local/grpc
export               PATH=$gRPC_ROOT/bin:$PATH
export    LD_LIBRARY_PATH=$gRPC_ROOT/lib64:$gRPC_ROOT/lib:$LD_LIBRARY_PATH
export       LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBRARY_PATH
export        LD_RUN_PATH=$LD_LIBRARY_PATH:$LD_RUN_PATH
```
`$ source ~/.bashrc`

### 可能遇到的问题
1. **‘TCP_USER_TIMEOUT’ was not declared in this scope**  
    修改 `src/core/lib/iomgr/socket_utils_common_posix.cc`，把第三十六行 `<netinet/tcp.h>` 替换成 `<linux/tcp.h>`。
    ```
    #ifdef GRPC_LINUX_TCP_H
    #include <linux/tcp.h>
    #else
    #include <linux/tcp.h>
    //#include <netinet/tcp.h>
    #endif
    ```
    参考 [include linux/tcp.h on Linux pre-glibc 2.17](https://github.com/grpc/grpc/pull/18729)

2. **libprofiler.so.0 库找不到**
    在 ~/.bashrc 里，修改环境变量 `LD_LIBRARY_PATH`，加入 `/usr/local/lib`。
    
3. **ldconfig: Can't create temporary cache file /etc/ld.so.cache~: Permission denied**
    ```
    $ sudo ldconfig
    ```
    这个问题可以不解决，因为没有 sudo 权限，不影响使用