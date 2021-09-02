1. 
进程 1 执行 `mysqld --initialize` 初始化到一个公共目录 `mysql_data`，然后进程 1 `mysqld_safe` 启动；  
此时，进程 2 不用初始化，直接启动，数据目录为公共目录 `mysql_data`；  
进程 1 能够直接登录；进程 2 尝试登录时，.sock 文件突然消失，管道关闭，登录失败，过段时间后进程 2 直接自己就退出了，查看日志显示：
```
2019-11-25T04:19:45.544852Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:46.545042Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:47.545250Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:48.545455Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:49.545631Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:50.545914Z 1 [ERROR] [MY-012574] [InnoDB] Unable to lock ./ibdata1 error: 11
2019-11-25T04:19:50.547922Z 1 [ERROR] [MY-012592] [InnoDB] Operating system error number 11 in a file operation.
2019-11-25T04:19:50.547984Z 1 [ERROR] [MY-012596] [InnoDB] Error number 11 means 'Resource temporarily unavailable'
2019-11-25T04:19:50.548062Z 1 [ERROR] [MY-012215] [InnoDB] Cannot open datafile './ibdata1'
2019-11-25T04:19:50.548148Z 1 [ERROR] [MY-012959] [InnoDB] Could not open or create the system tablespace. If you tried to add new data files to the system tablespace, and it failed here, you should now edit innodb_data_file_path in my.cnf back to what it was, and remove the new ibdata files InnoDB created in this failed attempt. InnoDB only wrote those files full of zeros, but did not yet use them in any way. But be careful: do not remove old data files which contain your precious data!
2019-11-25T04:19:50.548206Z 1 [ERROR] [MY-012930] [InnoDB] Plugin initialization aborted with error Cannot open a file.
2019-11-25T04:19:51.526133Z 1 [ERROR] [MY-010334] [Server] Failed to initialize DD Storage Engine
2019-11-25T04:19:51.526585Z 0 [ERROR] [MY-010020] [Server] Data Dictionary initialization failed.
2019-11-25T04:19:51.528575Z 0 [ERROR] [MY-010119] [Server] Aborting
2019-11-25T04:19:51.531011Z 0 [System] [MY-010910] [Server] /home/zhangrongrong/mysql2/install/bin/mysqld: Shutdown complete (mysqld 8.0.17-8)  Source distribution.
2019-11-25T04:19:51.562781Z mysqld_safe mysqld from pid file /home/zhangrongrong/mysql2/tmp/mysql.pid ended with return value of 1
```

> 两个进程都已在 my.cnf 配置 `skip_external_locking`，进程 1 登陆进去查看 `show variables like "%external%";`，显示已经跳过外部锁定：

![show variables](https://note.youdao.com/yws/api/personal/file/WEB12a7bb19f7aae6f2042b400b8f31be8d?method=download&shareKey=1f44b53dae7845f77fc1c140e462c2a9)

> 但是，两个进程不同时启动时，是可以正确跑的，也就是说，不能同时开两个进程读同一个数据库目录。


2.
既然配置了 `skip_external_locking`，进程 2 还是在尝试锁定 `./ibdata1` 时失败，进一步又去找了 `./ibdata1` 的路径配置（innobase提供了一些参数，可以使redo、undo、ibdata等文件放到公共目录 `mysql_data` 之外的地方），隔离两个进程系统表空间 `ibdata` 后再次尝试两边使用同一个数据目录登录 mysql。  
再次报错，进程 2 尝试锁定 `mysql.ibd`，这个文件属于表空间 `mysql`，且无法配置到其他目录。  
再次失败。


> 第 2 个问题其实还会产生其他的问题：比如隔离了两个进程 innobase 相关的系统表空间，进程 1 对数据路所做的修改(比如修改密码)，这个在其他进程是不可见的，事务日志号保存在公共目录，进程 2 启动后，发现日志号比自己超前，也会报错。

3.
最终注释掉 `os0file.cc` 中函数 `os_file_lock` 里：
```
/**
@AUTHOR (rrzhang, 张融荣)
 * 注释掉文件锁，可以实现多进程读同一个 data 目录功能。
 * The function of reading the same data directory by multiple processes can be realized by commenting out the file lock.
 */
#ifdef MULTI_MASTER_ZHANG_DEBUG
  int flag = 0;
#else
  int flag = fcntl(fd, F_SETLK, &lk);
#endif
```
调用系统文件锁的部分，就可以完成多进程对同一数据库目录的读。