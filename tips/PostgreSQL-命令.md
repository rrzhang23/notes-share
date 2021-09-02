不管安装方式怎样，在初始化后 data 目录下有配置文件 `postgresql.conf` 可以修改端口等参数。

yum、docker、源码安装后，启动服务和连接服务方式有所不同，具体如下：

#### yum 安装后相关命令
yum 安装 pg 后，pg 服务启动交给 systemctl 托管，切客户端连接时，需要切换到 postgres 用户。

**启动服务：**
```
$ psql --version
$ postgresql-setup initdb          // 初始化，data 目录为 /var/lib/pgsql/data
Initializing database ... OK 
$ systemctl start postgresql.service
# 会产生 "/var/run/postgresql/.s.PGSQL.5432" socket 文件
```

**客户端连接：**
```
$ psql postgres                    // 失败，要通过 postgres 用户登录
$ psql -U postgres                 // 同样失败  
psql: FATAL:  Peer authentication failed for user "postgres"
$ vim /var/lib/pgsql/data/pg_hba.conf  
# 修改：
local   all     all     Peer
# 至：
local   all     all     trust
$ systemctl restart postgresql.service
$ psql -U postgres
```

```
# 服务相关
systemctl start postgresql.service
systemctl status postgresql.service
systemctl stop postgresql.service
netstat -tunlp | grep postgres
```
#### docker 安装后相关命令
**启动服务：**
```
docker run --name postgres -e POSTGRES_PASSWORD=123456 -p 5432:5432 -d postgres:13
```
-name，指定创建的容器的名字；  

-e POSTGRES_PASSWORD=123456，设置环境变量，指定数据库的登录口令为 123456；

-p 54321:5432，端口映射将容器的5432端口映射到外部机器的54321端口；  

-d postgres:13，指定使用postgres:9.4作为镜像

参考 [Docker安装PostgreSQL](https://www.jianshu.com/p/75a7421cf787)

**客户端连接：**

至此至启动了服务端，需要在客户端访问，需要在使用安装目录下的 psql 命令访问：
```
./psql -h 127.0.0.1 -p 5432 postgres postgres
```

#### 源码安装后相关命令
假设安装参数：`./configure --prefix=/home/rrzhang/local/postgres-13`

**启动服务：**
```
mkdir /home/rrzhang/pgsql           
mkdri /home/rrzhang/pgsql/data              // 数据目录
mkdir /home/rrzhang/pgsql/log               // 日志目录
touch /home/rrzhang/pgsql/log/pg_server.log // 日志文件
# 初始化数据库
/home/rrzhang/local/postgres-13/bin/initdb -D /home/rrzhang/pgsql/data    
# 启动服务
/home/rrzhang/local/postgres-13/bin/pg_ctl -D /home/rrzhang/pgsql/data -l /home/rrzhang/pgsql/log/pg_server.log start
# 停止服务
/home/rrzhang/local/postgres-13/bin/pg_ctl -D /home/rrzhang/pgsql/data -l /home/rrzhang/pgsql/log/pg_server.log stop
```

**客户端连接：**
```
# 客户端登录
/home/rrzhang/local/postgres-13/bin/psql -h 127.0.0.1 -p 5432 postgres rrzhang
```