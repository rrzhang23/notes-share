### 初始化流程

MySQL 存储引擎作只要实现了 `percona-server/include/mysql/plugin.h` 定义的宏，就可以连接到存储层，该宏表现出类似 C++ 多态的特性。

innobase 在代码 `storage/innobase/handler/ha_innodb.h 、ha_innodb.cc` 里实现了插件的定义。具体参考：  
[MySQL InnoDB存储引擎启动过程源码分析](https://baijiahao.baidu.com/s?id=1644270736941739246&wfr=spider&for=pc)  
[Innodb 插件初始化](https://blog.csdn.net/innobase/article/details/53306934)

**《MySQL InnoDB存储引擎启动过程源码分析》** 中 `innobase_init()` 在 MySQL8.0 版本中为对应 `innodb_init()`，`innobase_start_or_create_for_mysql()` 对应 `srv_start`。

### data目录相关
ha_innodb 使用到的 data 目录定义为 `MySQL_datadir_path`，该值定义于 `fil0fil.h`。但是从日志打印的文件全路径来看，该值为 `./`，并不是完整的路径。mysqld 进程初始化和后续innobase进程貌似直接在 data 下运行了，所以用的 `./`。

日志里只有双写用到的文件 xb_doublewrite 是带全路径的，相关代码在 `buf0dblwr.cc::buf_parallel_dblwr_make_path()` 函数中，该函数调用了 `percona-server/mysys/my_symlink.cc::my_realpath()` 来获取全路径名。奇怪的是居然在 #ifdef _WIN32 开关里执行。