# 编译安装 MYSQL

> 参考：  
> 中文站：[www.mysqlzh.com](https://www.mysqlzh.com/doc/10.html)  
> 英文文档：[2.9 Installing MySQL from Source](https://dev.mysql.com/doc/refman/8.0/en/source-installation.html)  
> cmake 可选参数：[2.9.7 MySQL Source-Configuration Options](https://dev.mysql.com/doc/refman/8.0/en/source-configuration-options.html)  

## 准备

### 依赖
| 包名 | 版本 |
| --------- | --------- |
| MYSQL | 8.0.23 |
| GCC | > 5.3 |
| boost | > 1.73.0 |
| protobuf | 源码包自带，extra/protobuf 下 |

下载 boost 解压至 mysql-8.0.23 目录下：
```
- mysql-8.0.23
  - boost_1_73_0
    - boost
	  - accumulators
	  - algorithm
	  - core
  - client
  - cmake
  - CMakeLists.txt
  - components
  - ...
```

## 注意
cmake 时需要指定几个目录：`CMAKE_INSTALL_PREFIX`、`MYSQL_DATADIR`。  
另外 gcc 目录可以通过 `export /.../bin/gcc`，也可以通过传参数 `CMAKE_CXX_COMPILER`、`CMAKE_C_COMPILER` 给 cmake，这两个参数是 cmake 通用的，不是传给 MySQL 的。  

## 安装
我们指定以下几个目录及文件：
```
- mysql-8.0.23/mysql // 安装目录
- mysql-8.0.23/data  // data目录
- mysql-8.0.23/tmp   // tmp 目录，存放 .pid、.socks、log 等文件
- mysql-8.0.23/my.cnf
```

```
export CC=$HOME/.local/gcc-7.4.0/bin/gcc
export CXX=$HOME/.local/gcc-7.4.0/bin/g++
export LD_LIBRARY_PATH=/lib:/lib64:/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64
export LD_LIBRARY_PATH=$GCC_HOME/lib:$GCC_HOME/lib64:$GCC_HOME/libexec:$LD_LIBRARY_PATH


cmake ../ \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/mysql-8.0.23/mysql \
-DMYSQL_DATADIR=/home/zhangrongrong/mysql-8.0.23/data \
-DWITH_BOOST=../


nohup time make -j24 &
make install
```





## 初始化及使用
[MySQL 命令](tips/MySQL-命令.md) (https://rrzhang23.github.io/notes/#/tips/MySQL-%E5%91%BD%E4%BB%A4)

## 遇到的问题
#### 1. 
```
../runtime_output_directory/uca9dump: /lib64/libstdc++.so.6: version `CXXABI_1.3.9' not found (required by ../runtime_output_directory/uca9dump)
```

方法：在用户 bashrc 下添加：
```
export LD_LIBRARY_PATH=/lib:/lib64:/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64
export LD_LIBRARY_PATH=$GCC_HOME/lib:$GCC_HOME/lib64:$GCC_HOME/libexec:$LD_LIBRARY_PATH
```

#### 2. 
```
/bin/sh: /home/zhangrongrong/106.14.218.178/Percona-Share-Storage/percona-server/build/storage/tokudb/PerconaFT/xz/src/build_lzma/configure: Permission denied
```
方法：

```
chmod -R 775 /home/zhangrongrong/106.14.218.178/Percona-Share-Storage/percona-server/build/storage/tokudb/PerconaFT/xz/src/build_lzma
/home/zhangrongrong/CLionProjects/Percona-Share-Storage/percona-server/build-percona/storage/tokudb/PerconaFT/xz/src/build_lzma/configure
```
#### 3.
```
Cannot find appropriate system libraries for WITH_SSL=system.
```

方法：
```
yum install openssl-devel
或
apt-get install openssl
sudo apt-get install libssl-dev
```

#### 4. 
```
Could NOT find Curses (missing: CURSES_LIBRARY CURSES_INCLUDE_PATH) 
```

方法：
```
apt-get install libncurses5-dev
```

#### 5. 
```
Not building keyring_vault, could not find library: CURL
```

方法：
```
yum install curl
yum install curl-devel
```

#### 6. 有些时候会提示源码错误，自己进去源码改改好了

>大多是库没装引起的问题，装上就好了
