gcc 编译、预编译

make、cmake 安装软件时指定目录

cmake 怎样查找目录，.cmake文件怎么使用，.config又是什么文件，

make 指定那些预编译、链接参数;cmake又是怎么指定的。

make、cmake 指定 c++ 版本问题，静态、动态链接库问题

clang libc++、libc++abi

cmake 命令整理

cmake find_package

jsoncpp

-D 和 add_compile_options

C++ lamda表达式

ceph 笔记

std::strto*** 和 std::sto** 比较

腾讯 blade

除了 writer 方式，用另一种 document 构建 json

类内 const 定义

.clion.source.upload.marker

```
percona-server/CMakeList.txt :
1262 : IF(NOT WITHOUT_SERVER)
...
1276 : CONFIGURE_PLUGINS() : 
        percona-server\cmake\plugin.cmake
        
最后会加载所有storage 下的CMakeList.txt
```

测试rpc连续读写 >1MB 的数据

对于innobase pread 打印日志，查看到底调用了哪个 pread