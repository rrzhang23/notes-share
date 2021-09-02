## 1. yum 安装

安装依赖 `postgresql postgresql-libs postgresql-server`  

`postgresql` 客户端  
`postgresql-libs` 客户端依赖的库  
`postgresql-server` 服务端   
`postgresql-devel` 可以不安装


`yum search postgres` 可以查看几个包的区别 

```
# 安装, 会一并安装依赖 postgresql postgresql-libs，同时会附带生成 postgres 用户，'cat /etc/passwd' 查看
yum install postgresql-server

# 卸载
yum remove postgresql postgresql-server  postgresql-libs
sudo yum history list postgresql
sudo yum history undo 14
userdel -r postgres
rm -rf /var/lib/pgsql
```

参考 [linux centos7下通过yum安装默认的postgresql](https://blog.csdn.net/feinifi/article/details/96474115)

## 2. docker 安装
访问 hub.docker.com 找到需要的库

**拉取镜像：**
```
docker pull postgres:13
```


## 3. 源码安装
```
# 安装部分依赖库
sudo yum install readline readline-devel zlib zlib-devel bison flex
curl -OL https://github.com/postgres/postgres/archive/REL_13_RC1.tar.gz
tar -xf REL_13_RC1.tar.gz
cd postgres-REL_13_RC1
./configure --prefix=/home/rrzhang/local/postgres-13
make -j32 && make install
```
参考 [PostgreSQL源码编译安装与配置](https://blog.csdn.net/Linjingke32/article/details/80393576)

##### 遇到的错误
`gcc: error: ./specs: Is a directory`
这是由于 postgres 中目录 `specs` 干扰了 gcc 执行。或者和 gcc 相关的环境变量中有双冒号 `::`

可以这么测试：
```
$ mkdir -p test/specs
$ cd test
$ gcc --version
gcc: error: ./specs: Is a directory
```
或者:
```
$ echo $LD_LIBRARY_PATH
$ echo $LIBRARY_PATH
$ echo $LD_RUN_PATH
```
某个变量后面极有可能有双冒号 `::`。

**解决方案：**  
把环境变量中双冒号去掉，可使用以下脚本，只对当前shell有效：
```
LD_LIBRARY_PATH=$(echo $LD_LIBRARY_PATH | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
LIBRARY_PATH=$(echo $LIBRARY_PATH | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
LD_RUN_PATH=$(echo $LD_RUN_PATH | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
```


参考 [cc: error: ./specs: Is a directory](https://github.com/raphlinus/pulldown-cmark/issues/122)

参考 ['specs' directory interferes with the operation of gcc in configure scripts.](https://gcc-help.gcc.gnu.narkive.com/8AbbmxJN/specs-directory-interferes-with-the-operation-of-gcc-in-configure-scripts)