
### how to run 
##### server
```
cd extra/multi_master/remote_fun
mkdir build && cd build
cmake .. && make -j32
```

##### client
```
mkdir build && cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/mysql/local/mysql80 -DMYSQL_DATADIR=/home/zhangrongrong/mysql/data -DWITH_BOOST=/home/zhangrongrong/.local/boost_1_69_0 -DWITHOUT_ROCKSDB=1 -DWITHOUT_TOKUDB=1 ~/mysql/local/mysql80/bin/*
make remote_fun -j32 && make -j32 && make install
```

### 修改/增加的文件：
1. 增加：  
    ```
    extra/multi_master/remote_fun
    ```
2. 修改：  
    ```
    CMakeList.txt  
    mysys/CMakeList.txt  
    mysys/[\*.h|\*.cc]  
    sql/CMakeList.txt  
    sql/sql_thd_api.cc  
    sql/sql_thd_internal_api.cc  
    sql/dd/upgrade_57/upgrade.cc  
    storage/innobase/CMakeList.txt  
    storage/innobase/innodb.cmake  
    storage/innobase/buf/buf0dump.cc  
    storage/innobase/clone/clone0api.cc  
    storage/innobase/clone/clone0clone.cc  
    storage/innobase/clone/clone0copy.cc  
    storage/innobase/handler/ha_innodb.cc  
    storage/innobase/include/srv0start.h  
    storage/innobase/os/os0file.cc  
    storage/innobase/srv/srv0start.cc  
    storage/perfschema/unittest/CMakeLists.txt  
    utilities/CMakeLists.txt 
    ```
    
> 所有的  CMakeList.txt 修改都用 `# START MULTI_MASTER_ZHANG`、`# END MULTI_MASTER_ZHANG`包裹；所有的 `.h`、`.cc` 代码都用 `#ifdef MULTI_MASTER_ZHANG*` 包裹。

### 根目录 CMakeList.txt
`NO_DEBUG_REMOTE_SINGLE` 用于控制 remote_fun 是否单独编译。  
```
  set(GRPC_ROOT $ENV{HOME}/.local/grpc-v1.23.x)
  include(${PROJECT_SOURCE_DIR}/extra/multi_master/remote_fun/cmake/grpc.cmake)
```
需要把 grpc 编译进去，grpc 的安装目录可能需要调整，并且 path_to_grpc/bin/ 下的 `*c++*` 这样的文件需要用 `*cpp*` 软连接代替，因为 mysql make 时，`+` 符号会有问题，编译不进去。  
protobuf 不需要连接 linux 的安装目录，mysql 自己带了 protobuf 的模块。

### remote_fun 模块
##### extra/multi_master/remote_fun/CMakeList.txt
`if (NOT NO_DEBUG_REMOTE_SINGLE)` 判断是否是 server 段单独编译 remote_fun 模块，单独编译需要添加一些头文件以及连接一些系统库，且 protobuf 使用不了 mysql 的了。  
还有用于日志的宏，在文件里做了注释。

### os0file.cc
map_fd : 开始 debug 的时候，在本地、远程都有同样的文件，这个 map 同于存储 `本地 fd 和远程 fd` 的对应；后来在 实现的时候，文件只写远程，这个 map 存储`远程 fd 和远程 fd` 的对应。  

map_path ： 存储远程 fd 和远程 path 的对应。

set_dir ： 存储远程打开的远程 dir。

path_log ：mysql 运行时的日志输出目录。

指针 `remote::RemoteClient *remote_client = 0;` 声明在 `srv0start.cc` 里，并且在 srvstart.cc::srv_start() 里初始化。

### mysys
由于 os0file.cc 只是针对底层储存层的文件读写，上层 mysys 里同样有文件的读写，所以 mysys 的结构和os0file.cc 里差不多，变量都以 `_mysys` 结尾。  
同样， `remote::RemoteClient *remote_client_mysys = 0;` 在 `mysys/my_static.cc` 里初声明，在 `mysys/my_init.cc::my_init()` 里初始化。

### 日志目录 && server 运行时的目录
所有的需要修改的目录都声明在 `extra/multi_master/include/multi_macro.h` 里。


### file_should_be_local 函数：
安装时会进行一些检查，也会读写文件，这时候的文件需要放在本地，详见 `extra/multi_master/include/multi_macro.h`。

