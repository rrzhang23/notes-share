结构：
~~~
- LAMP
  - mysql-8.0.25    // mysql 安装目录
  - mysql-data      // mysql data、tmp 存放目录
    - data          // mysql data
    - tmp           // mysql tmp
    my.cnf
  - php-7.3.28
  - apache-2.4.48
~~~
~~~cpp
// apache
cd ~/.local/src/LAMP
wget https://mirrors.bfsu.edu.cn/apache//httpd/httpd-2.4.48.tar.gz
wget http://archive.apache.org/dist/apr/apr-util-1.5.2.tar.gz
wget http://archive.apache.org/dist/apr/apr-1.5.2.tar.gz
tar -xf httpd-2.4.48.tar.gz
tar -xf apr-util-1.5.2.tar.gz
tar -xf apr-1.5.2.tar.gz
mv apr-util-1.5.2 apr-util
mv apr-1.5.2 apr
mv apr apr-util httpd-2.4.48/srclib/

// pcre
cd ~/.local/src/LAMP
wget https://jaist.dl.sourceforge.net/project/pcre/pcre/8.40/pcre-8.40.tar.gz
tar -xf pcre-8.40.tar.gz
cd pcre-8.40
./configure --prefix=/home/zhangrongrong/.local/LAMP/pcre-8.40
make -j32
make install

// apache
cd ~/.local/src/LAMP
cd httpd-2.4.48
./configure --prefix=/home/zhangrongrong/.local/LAMP/apache-2.4.48 -with-pcre=/home/zhangrongrong/.local/LAMP/pcre-8.40/bin/pcre-config -with-included-apr
make -j32
make install
~~~