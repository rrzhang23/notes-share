结构：
~~~
- LNMP
  - mysql-8.0.25    // mysql 安装目录
  - mysql-data      // mysql data、tmp 存放目录
    - data          // mysql data
    - tmp           // mysql tmp
    my.cnf
  - php-7.3.28
  - nginx-1.20.1
~~~
~~~cpp
// oniguruma
cd ~/.local/src/LNMP
wget https://github.com/kkos/oniguruma/archive/v6.9.4.tar.gz -O oniguruma-6.9.4.tar.gz
tar -zxf oniguruma-6.9.4.tar.gz
cd oniguruma-6.9.4
./autogen.sh
./configure --prefix=/home/zhangrongrong/.local/oniguruma-6.9.4
make -j32 && make install
ln -s /home/zhangrongrong/.local/oniguruma-6.9.4/ /home/zhangrongrong/.local/LNMP/oniguruma
export PKG_CONFIG_PATH=/home/zhangrongrong/.local/LNMP/oniguruma/lib/pkgconfig:$PKG_CONFIG_PATH

// libzip
cd ~/.local/src/LNMP
wget https://libzip.org/download/libzip-1.8.0.tar.xz
xz -dk -T 32 libzip-1.8.0.tar.xz
tar -xf libzip-1.8.0.tar
cd libzip-1.8.0/
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/libzip-1.8.0 ..
make -j32 && make install
ln -s /home/zhangrongrong/.local/libzip-1.8.0/ /home/zhangrongrong/.local/LNMP/libzip
export PKG_CONFIG_PATH=/home/zhangrongrong/.local/libzip-1.8.0/lib/pkgconfig:$PKG_CONFIG_PATH

or

wget https://libzip.org/download/libzip-1.2.0.tar.xz
xz -dk -T 32 libzip-1.2.0.tar.xz
tar -xf libzip-1.2.0.tar
cd libzip-1.2.0/
./configure --prefix=/home/zhangrongrong/.local/libzip-1.2.0
make -j32 && make install
ln -s /home/zhangrongrong/.local/libzip-1.2.0/ /home/zhangrongrong/.local/LNMP/libzip
export PKG_CONFIG_PATH=/home/zhangrongrong/.local/libzip-1.2.0/lib/pkgconfig:$PKG_CONFIG_PATH



// php
cd ~/.local/src/LNMP
wget https://www.php.net/distributions/php-7.3.28.tar.gz
tar -xf php-7.4.20.tar.gz
cd php-7.4.20/
./configure --prefix=/home/zhangrongrong/.local/php-7.4.20 --with-curl --enable-gd --enable-mbstring --with-openssl --with-pdo-mysql=/home/zhangrongrong/.local/LNMP/mysql --with-zip --with-zlib --enable-intl --enable-ftp --enable-bcmath --enable-exif
make -j32 && make install
ln -s /home/zhangrongrong/.local/php-7.4.20/ /home/zhangrongrong/.local/LNMP/php

// mysql
cd ~/.local/src/LNMP

mkdir ~/.local/LNMP/mysql-8.0.25
mkdir ~/.local/LNMP/mysql-data
mkdir ~/.local/LNMP/mysql-data/data
mkdir ~/.local/LNMP/mysql-data/tmp

wget https://github.com/mysql/mysql-server/archive/refs/tags/mysql-8.0.25.tar.gz
wget https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz
tar -xf mysql-8.0.25.tar.gz
tar -xf boost_1_73_0.tar.gz
mv boost_1_73_0 mysql-server-mysql-8.0.25/
cd mysql-server-mysql-8.0.25/
mkdir build
cd build/
cmake ../ -DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/LNMP/mysql-8.0.25 -DMYSQL_DATADIR=/home/zhangrongrong/.local/LNMP/mysql-data/data -DWITH_BOOST=../
make -j32
make install

cd mkdir ~/.local/LNMP/mysql-data
touch my.cnf
vim my.cnf


// nginx
cd ~/.local/src/LNMP
wget http://nginx.org/download/nginx-1.20.1.tar.gz
tar -xf nginx-1.20.1.tar.gz
cd nginx-1.20.1
./configure --prefix=/home/zhangrongrong/.local/nginx-1.20.1
make -j32
make install
~~~


### ~/.local/LNMP/mysql-data/my.cnf:
~~~cpp
[client]
port = 33061
socket = /home/zhangrongrong/.local/LNMP/mysql-data/tmp/mysql.sock

# [mysqld-8.0] # 只有 8.0.x 版本才会用到这个配置，[mysqld-8.0][mysqld-5.7] 可以并存。测试好像没用，还不如直接[mysqld]
[mysqld]
port = 33061
socket = /home/zhangrongrong/.local/LNMP/mysql-data/tmp/mysql.sock
basedir                     = /home/zhangrongrong/.local/LNMP/mysql-8.0.25
datadir                     = /home/zhangrongrong/.local/LNMP/mysql-data/data
plugin-dir                  = /home/zhangrongrong/.local/LNMP/mysql-8.0.25/lib/plugin

log-error                   = /home/zhangrongrong/.local/LNMP/mysql-data/tmp/mysql.err
pid-file                    = /home/zhangrongrong/.local/LNMP/mysql-data/tmp/mysql.pid

lc_messages_dir             = /home/zhangrongrong/.local/LNMP/mysql-8.0.25/share/
character-set-server        = UTF8MB4    # UTF8MB4 是 utf8 别名
~~~