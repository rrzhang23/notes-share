# MySQL 命令

## 非 root 启动 mysql
> 参考：[5.8.3 在 Unix 上运行多个 MySQL 实例](https://dev.mysql.com/doc/refman/8.0/en/multiple-unix-servers.html)

mysql 默认运行在 mysql 用户下，当想在同一机器，或者非 root 用户目录下使用 mysql 时，需要修改指定的几个变量：  
1. port, 端口，默认 3306，可以指定其他端口
2. .sock 文件，默认为 /tmp/mysql.sock
3. data 目录。

## 几个进程
1. mysql，客户端程序，通常用 `mysql -uroot -p''` 这样的命令连接 mysql 服务。
2. mysqld，服务端。
3. mysqld_safe，启动脚本，一般不会直接通过 mysqld 启动服务，而是通过这个脚本启动。它会在 mysqld 挂掉时拉起 mysqld 进程。

## 给 mysqld 传参
通常 mysqld 启动时需要上面提到的几个参数：端口、.sock 文件、data 目录。有几个方法：
1. 在编译之前就通过 cmake 指定好这些参数 `-DMYSQL_TCP_PORT=3306`、`-DMYSQL_UNIX_ADDR=somepath/mysql.sock`。
2. my.cnf 文件
3. 命令行传参
4. shell 中直接 export 这些变量。
> 参考：[5.8.3 在 Unix 上运行多个 MySQL 实例](https://dev.mysql.com/doc/refman/8.0/en/multiple-unix-servers.html)

#### 其他有用的变量：
```
--log-error, error log 输出目录，初始化 data 目录时，初始密码就在这个文件里
--pid-file
// MySQL pid 文件记录的是当前 mysqld 进程的 pid，pid 亦即 Process ID。
// 未指定 pid 文件时，pid 文件默认名为 主机名.pid，存放的路径在默认 MySQL 的数据目录。
// 通过mysqld_safe启动MySQL时，mysqld_safe会检查pid文件，如果pid文件不存在，不做处理；如果文件存在，且pid已占用则报错；如果文件存在且pid未占用，则删除pid文件。
// mysqld 启动后会通过 create_pid_file 函数新建 pid 文件,通过getpid() 获取当前进程 pid 并将 pid 写入 pid 文件。
// 因此,通过 mysqld_safe 启动时, MySQL pid 文件的作用是:防止同一个数据库被启动多次。
```

## 启动 mysqld
mysqld 使用前需要通过 `mysqld  --initialize` 初始化 data 目录，然后再启动 mysqld。

#### 1. 通过 my.cnf
> 参考：  
> [4.2.2.2 使用选项文件](https://dev.mysql.com/doc/refman/8.0/en/option-files.html)

可通过 ` /home/zhangrongrong/mysql-8.0.23mysql/bin/mysqld --verbose --help |grep -A 1 'Default options'` 查看默认加载 `my.cnf` 的路径：  
`/etc/my.cnf /etc/mysql/my.cnf /home/zhangrongrong/mysql-8.0.23/mysql/etc/my.cnf ~/.my.cnf`。

```
[client]
port = 33061
socket = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.sock

# [mysqld-8.0] # 只有 8.0.x 版本才会用到这个配置，[mysqld-8.0][mysqld-5.7] 可以并存。测试好像没用，还不如直接[mysqld]
[mysqld]
port = 33061
socket   = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.sock
basedir                     = /home/zhangrongrong/mysql-8.0.23/mysql
datadir                     = /home/zhangrongrong/mysql-8.0.23/data
plugin-dir                  = /home/zhangrongrong/mysql-8.0.23/mysql/lib/plugin

log-error                   = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.err
pid-file                    = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.pid

lc_messages_dir             = /home/zhangrongrong/mysql-8.0.23/mysql/share/
character-set-server        = UTF8MB4	# UTF8MB4 是 utf8 别名
```

```
cd ~/mysql-8.0.23
// 初始化，密码写在 /home/zhangrongrong/mysql-8.0.23/tmp/mysql.err 里
mysql/bin/mysqld --defaults-file=~/mysql-8.0.23/my.cnf --initialize --user=zhangrongrong
or
mysql/bin/mysqld --defaults-file=~/mysql-8.0.23/my.cnf --initialize --console --user=zhangrongrong
// 启动
mysql/bin/mysqld_safe --defaults-file=~/mysql-8.0.23/my.cnf --user=zhangrongrong &
// 连接
mysql/bin/mysql --defaults-file=~/mysql-8.0.23/my.cnf -uroot -p'********'
mysql/bin/mysql --defaults-file=~/mysql-8.0.23/my.cnf --host=localhost --port=33061 --user=root --password='********'
mysql/bin/mysql --defaults-file=~/mysql-8.0.23/my.cnf -hlocalhost -P33061 -uroot -p'********'
mysql/bin/mysql --defaults-file=~/mysql-8.0.23/my.cnf -h localhost -P 33061 -u root -p'********'
// 修改密码
mysql> alter user 'root'@'localhost' identified by '123456';
// 关闭
mysql/bin/mysqladmin --defaults-file=~/mysql-8.0.23/my.cnf -uroot -p123456 shutdown
```

#### 2. 通过命令行传参
> 参考：  
> [4.2.2.1 使用命令行上的选项](https://dev.mysql.com/doc/refman/8.0/en/command-line-options.html)  
> [4.2.2.3 影响选项文件处理的命令行选项](https://dev.mysql.com/doc/refman/8.0/en/option-file-options.html)

类似 my.cnf 方式，只不过通过 `--port` 等方式把参数传给 mysqld，和写在 my.cnf 里没有区别。

## 遇到的问题

### 1. 初始化时：
```
2019-11-07T06:08:30.833089Z 0 [ERROR] [MY-010338] [Server] Can't find error-message file '/home/zhangrongrong/mysql-8.0.23/mysql/share/errmsg.sys'. Check error-message file location and 'lc-messages-dir' configuration directive.
```
这个问题产生的原因是，我指定的 basedir(同安装目录 `CMAKE_INSTALL_PREFIX`) 为 `/home/zhangrongrong/mysql-8.0.23/mysql80`，而不是正常的 `/home/zhangrongrong/mysql-8.0.23/mysql`。  
方法：在 my.cnf 里添加 `lc-messages-dir=/home/zhangrongrong/mysql-8.0.23/mysql80/share/`

## my.cnf 其他参数
```
[client]
port = 33061
socket = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.sock

# [mysqld-8.0] # 只有 8.0.x 版本才会用到这个配置，[mysqld-8.0][mysqld-5.7] 可以并存。测试好像没用，还不如直接[mysqld]
[mysqld]
port = 33061
socket   = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.sock
basedir                     = /home/zhangrongrong/mysql-8.0.23/mysql
datadir                     = /home/zhangrongrong/mysql-8.0.23/data
plugin-dir                  = /home/zhangrongrong/mysql-8.0.23/mysql/lib/plugin

log-error                   = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.err
pid-file                    = /home/zhangrongrong/mysql-8.0.23/tmp/mysql.pid
character-set-server        = UTF8MB4	# UTF8MB4 是 utf8 别名





# lc_messages_dir             = /home/zhangrongrong/mysql-8.0.23/mysql/share/
# character_sets_dir          = /home/zhangrongrong/mysql-8.0.23/share/charsets

# innnodb 一些路径可以不配置在 data 目录，可以自己指定其他任意目录：
# innodb_data_home_dir        = /home/zhangrongrong/other_path.../innodb_data
# innodb_directories          = /home/zhangrongrong/other_path.../innodb_directories
# innodb_log_group_home_dir   = /home/zhangrongrong/other_path.../innodb_log_group
# innodb_temp_tablespaces_dir = /home/zhangrongrong/other_path.../#innodb_temp/
# innodb_tmpdir               = /home/zhangrongrong/other_path.../innodb_tmp
# innodb_undo_directory       = /home/zhangrongrong/other_path.../innodb_undo_directory
# 临时表空间：
# tmpdir                      = /home/zhangrongrong/other_path.../tmpdir
# slave_load_tmpdir           = /home/zhangrongrong/other_path.../slave_load_tmpdir

# skip-external-locking       
# skip_external_locking
# user = zhangrongrong
```
