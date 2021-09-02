### 使用标准输出
`std::cout` 是可输出到 `data/` 目录下 `.err` 后缀的日志文件里的（至少 os0file.cc 中可以）。

### 使用自定义 logger
自定义 log 工具 logger.h logger.cc 放到 innobase/os 目录下，编辑 `innobase/CMakeList.txt` ，添加 logger.cc 至 `SET(INNOBASE_SOURCES)` 中。  

在 `os0file.cc`代码中对应位置使用宏定义语句就可以打印日志。位置由 `logger.cc` 中指定。

### 修改过的文件
##### include/
```
添加 logger.h logger.cpp
```

##### mysys
```
修改 CMakeList.txt
修改 my_symlink.cc
my_open.cc
my_create.cc
```

##### storage/
```
修改 innobase/buf/buf0dblwr.cc
修改 innobase/os/os0file.cc
修改 innobase/handler/ha_innodb.cc
修改 innobase/CMakeList.txt
```



### 存在问题
函数 os_file_create_simple_no_error_handling_func 中使用该 LOG 语句时（大概3686行左右），编译后，mysqld_safe 进程会起不来，报错：xb_doublewrite 文件不能创建。




#### 1
1. os_file_fsync_posix
2. execute

这两个函数里不能使用 LOG_FUNC 

#### 2
下面几个函数需要将日志打印调用放在调用库函数上面，不然弄明奇妙的错误。
```
os_file_create_directory() {
LOG_FUNC << "mkdir" << std::endl;
      int rcode = mkdir(pathname, 0770);
}
```

```
os_file_create_simple_no_error_handling_func() {
LOG_FUNC << "open" << std::endl;
LOG_NORMAL << name << std::endl;
LOG_SIMPLE << name << std::endl;
      file.m_file = ::open(name, create_flag, os_innodb_umask);
}
// 若 LOG 放在 open 下面，
// mysqld_safe 进程会起不来，报错：xb_doublewrite 文件不能创建。
```

```
os_offset_t os_file_get_size(pfs_os_file_t file) {
LOG_FUNC << "lseek" << std::endl;
    ...
}

// 若 LOG_FUNC 放在下面，tpcc跑的时候有问题。
```