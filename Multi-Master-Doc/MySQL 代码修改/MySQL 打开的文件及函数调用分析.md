#### 1. 函数调用频次分析
```
my_sys 里调用的库函数，除了 my_open()::open、my_create()::open， 其他的没有统计
os0file 里面一共 26 个库函数（人工找的，可能不止），下面结果多出来的是 my_open()::open、my_create()::open。
结果为 1 表示没有调用，实际上 pread、pwrite、fsync，是调用了的，但是日志打印会导致 mysqld 进程出错，就没打出来。

putc 1
posix_fadvise 1
fread 1
rmdir 1
fprintf 1
fputs 1
rewind 1
pwrite 1        # 用到了，实际不止 1
closedir 1
statvfs 1
fsync 1         # 用到了，实际不止 1
sendfile 1
pread 1         # 用到了，实际不止 1
rename 2
mkdir 3
ftruncate 4
opendir 8
fdopen 11
my_open()::open 23
unlink 27
fallocate 63
fcntl 102
close 114
open 115
my_create()::open 118
readdir 161
stat 580
lseek 1141
```

#### 2. 计算层 my_sys 打开的文件
> 参考   
>
> binlog：  
> &#8195;&#8195;[
MySQL:简单记录删除binary log的接口](http://blog.itpub.net/7728585/viewspace-2649839/)
```
./auto.cnf                  # 存储了server-uuid的值
./binlog.000001             # binlog 文件
./binlog.index              # binlog 的索引文件
./binlog.index_crash_safe   # 和 ./binlog.~rec~ 都是用于删除 binlog 的
./binlog.~rec~
// general_log 记录 sql 语句
./mysql/general_log.CSM     
./mysql/general_log.CSV
./mysql/general_log_212.sdi
// 慢查询日志
./mysql/slow_log.CSM
./mysql/slow_log.CSV
./mysql/slow_log_213.sdi
// 未知 
/home/zhangrongrong/mysql/data/mysqld_tmp_file_case_insensitive_test.lower-test
// 下面两个是启动时读取安装目录下的文件
/home/zhangrongrong/mysql/local/mysql80/share/charsets/Index.xml
/home/zhangrongrong/mysql/local/mysql80/share/english/errmsg.sys
// pid socket文件
/home/zhangrongrong/mysql/tmp/mysql.pid         
ca-key.pem                  # ssl 连接相关
ca.pem
client-cert.pem
client-key.pem
private_key.pem
public_key.pem
server-cert.pem
server-key.pem
```

#### 3. innobase 层 os0file.cc 打开的文件
> 参考   
>
> 临时表空间池：  
> &#8195;&#8195;[《MySQL · 引擎特性 · 临时表那些事儿》](http://mysql.taobao.org/monthly/2019/04/01/)  
> &#8195;&#8195;[MySQL8.0 - 新特性 - 临时表改进](https://blog.csdn.net/weixin_34416754/article/details/89544991)  

> 临时表空间：  
> &#8195;&#8195;[从MYSQL的ibtmp1文件太大说起](https://www.cnblogs.com/gjc592/p/11358935.html)  
>
> undo_1_trunc.log ：  
> &#8195;&#8195;[MySQL · 5.7特性 · 在线Truncate undo log 表空间](http://mysql.taobao.org/monthly/2014/11/06/)  
> &#8195;&#8195;[转自： http://mysqllover.com/?p=1051&utm_source=tuicool
MySQL 5.7 新特性：在线truncate undo log文件](http://blog.sina.com.cn/s/blog_54904e1c0102vikq.html)  
> 
> sys 库:   
> &#8195;&#8195;[MySQL中的sys系统数据库是干嘛的](https://www.cnblogs.com/mojiexiaolong/p/6978540.html)
>
>


&#8195;&#8195;
```
// 《MySQL · 引擎特性 · 临时表那些事儿》 “临时表“小节：
/ ***
    在MySQL 8.0后，磁盘临时表的数据单独放在 Session 临时表空间池
    (#innodb_temp目录下的ibt文件)里面，
    临时表的undo放在global的表空间ibtmp1里面。
    */
./#innodb_temp/temp_1.ibt   # 临时表空间池文件
./#innodb_temp/temp_10.ibt
./#innodb_temp/temp_2.ibt
./#innodb_temp/temp_3.ibt
./#innodb_temp/temp_4.ibt
./#innodb_temp/temp_5.ibt
./#innodb_temp/temp_6.ibt
./#innodb_temp/temp_7.ibt
./#innodb_temp/temp_8.ibt
./#innodb_temp/temp_9.ibt
./ib_logfile0               # redo log
./ib_logfile1
./ib_logfile101
./ibdata1                   # 系统共享表空间
./ibtmp1                    # 临时表空间
./sys/sys_config.ibd        # 系统的元数据信息
./tpcc/CUSTOMER.ibd         # tpcc 负载用到的表空间
./tpcc/DISTRICT.ibd
./tpcc/HISTORY.ibd
./tpcc/ITEM.ibd
./tpcc/NEW_ORDER.ibd
./tpcc/OORDER.ibd
./tpcc/ORDER_LINE.ibd
./tpcc/STOCK.ibd
./tpcc/WAREHOUSE.ibd
./undo_001                  # undo log
./undo_002
./undo_1_trunc.log          # truncate undo log 的日志，用于恢复
./undo_2_trunc.log
/home/zhangrongrong/mysql/data/xb_doublewrite   # 双写缓冲区对应的文件
mysql.ibd                   # 未知
```