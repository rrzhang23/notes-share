参考：  
[官方文档](http://llvm.org/docs/GettingStarted.html#compiling-the-llvm-suite-source-code)  
[官方 Building LLVM with CMake](https://llvm.org/docs/CMake.html)  
[llvm+clang编译安装](https://www.cnblogs.com/Long-w/p/6345028.html)
[结构化编译器前端 Clang 介绍](https://www.ibm.com/developerworks/cn/opensource/os-cn-clang/)




[使用 Clang Tools —— 概述](https://blog.csdn.net/qq_23599965/article/details/91039691)

## 1. 介绍


## 2. 安装及使用
两种方式：  
1. 直接下载二进制包
2. 编译安装

llvm 和 clang 必须安装；  
clang-tools-extra, libcxx, libcxxabi, libunwind, lldb, compiler-rt, lld, polly, debuginfo-tests 可选。
#### bin 二进制包：
deepin 可以直接使用 ubuntu 版本的二进制包，解压、配置环境变量即可，clion 也无需其他多余的配置。

#### 编译安装：
[编译安装 clang](http://note.youdao.com/noteshare?id=1cdae4946689987b45aa38a87ff32324)

> 遇到的问题：
> <scsi/scsi.h> 问题，可参考 [编译安装 gcc](http://note.youdao.com/noteshare?id=0d517a03aea7267d4c3f726209a812fc)
> 
> 


#### 问题
编译时，老是链接 /usr/bin 下面 gcc 的库，就很烦，尤其是低版本的 gcc 根本不能用。  
解决：  
clang 添加参数：
```
clang++ --gcc-toolchain=/home/zhangrongrong/.local/gcc-7.4.0 -v
```

CmakeList.txt 加入：
```
add_compile_options(--gcc-toolchain=/home/zhangrongrong/.local/gcc-7.4.0)
```


#### 环境变量设置
```
# C & C++#
export     MY_GCC_VERSION=7.4.0
export           GCC_HOME=$HOME/.local/gcc-$MY_GCC_VERSION
# export               PATH=$GCC_HOME/bin:$PATH
export    LD_LIBRARY_PATH=$GCC_HOME/libexec/gcc/x86_64-unknown-linux-gnu/$MY_GCC_VERSION:$GCC_HOME/lib/../lib64:$GCC_HOME/lib:$GCC_HOME/lib64:$GCC_HOME/libexec # :$LD_LIBRARY_PATH
export       LIBRARY_PATH=$GCC_HOME/libexec/gcc/x86_64-unknown-linux-gnu/$MY_GCC_VERSION:$GCC_HOME/lib/../lib64:$GCC_HOME/lib:$GCC_HOME/lib64:$GCC_HOME/libexec # :$LIBRARY_PATH
export        LD_RUN_PATH=$GCC_HOME/libexec/gcc/x86_64-unknown-linux-gnu/$MY_GCC_VERSION:$GCC_HOME/lib/../lib64:$GCC_HOME/lib:$GCC_HOME/lib64:$GCC_HOME/libexec # :$LD_RUN_PATH
export     C_INCLUDE_PATH=$GCC_HOME/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$GCC_HOME/include:$CPLUS_INCLUDE_PATH


# clang && clang++
export         CLANG_HOME=$HOME/.local/clang+llvm-7.1.0
export               PATH=$CLANG_HOME/bin:$PATH
export                 CC=$CLANG_HOME/bin/clang
export                CXX=$CLANG_HOME/bin/clang++
```
> clang 需要连接到 gcc 动态库去