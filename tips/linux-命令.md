#### 修改配置文件
```
vim ~/.bashrc
source ~/.bashrc

visudo  // 编辑 sudoers 文件，添加用户到sudoers

sudo ldconfig
```

#### 解压、压缩
```
tar 只是打包，xz、zip 才是压缩

# tar :
# -c （create，创建，压缩）,-x （extract，抽取，解压）
# 解压
tar -xf something.tar -C /toPath
# 打包
tar -rf something.tar somedir/ somefile


# xz, -T（多核）, -z(压缩), -d(解压), -k(保留源文件)
# 32 核解压
xz -dk -T 32 something.xz
# 32 核压缩
xz -z -T 32 something.tar

zip -r something.zip something/
unzip something.zip -d /path
```
[更多参考](https://www.cnblogs.com/manong--/p/8012324.html  
https://blog.csdn.net/u013439115/article/details/77935602)

#### 文件操作
```
// 文件
mkdir [目录名]  // 新建文件夹
touch [文件名]  // 新建文件
rm -rf [文件或目录]

// 拷贝 protobuf 下所有文件至 .local 中 （-r 表示文件）
sudo cp -r /home/rrzhang/.local/protobuf /home/temp/.local

sudo chmod -R 775 [目录名或文件名]   // 修改读写权限，-R 表示递归修改
sudo chown -R [用户名] [目录名或文件名]
sudo chgrp -R [用户组名] [目录名或文件名]

du -h   // 显示文件及其子文件夹大小
du -sh  // 显示文件夹大小

// 远程上传、下载文件，前面是源文件，后面是目的目录
scp [本地文件] temp@47.102.100.148:[远程目录]
scp temp@47.102.100.148:[远程文件] [本地目录]

// 当前文件夹下查找
find  . -name ‘somefile'
// 递归删除
find  . -name  '*.exe' -type  f -print -exec  rm -rf  {} \;
// 输出文件最后几行数据
cat nohup.out | tail -n 3
```
> 参考 [linux下递归删除目录下所有指定类型文件](https://blog.csdn.net/u014515854/article/details/80069063)

#### 进程
```
ps -ef | grep some | grep -v grep
kill -9 pid

# 显示包含 'main_' 的进程，并全部杀死
ps -ef | grep main_instance | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep main_lock_service | grep -v grep | awk '{print $2}' | xargs kill -9
```

#### 下载
```
// -P 指定目录，-O 指定文件名
wget -P /path -O filename url
```

#### xshell上传下载
上传 rz  
下载 sz

#### 后台运行
1.  
`nohup time ./test.sh &`
2.  
`(./test.sh &)`

查看 nohup 加入的后台程序：  
`jobs`
从后台切换到前台
`fg [number]`

#### vim
清空当前文档  
`:%d`

#### tcpdump
```
sudo tcpdump -S -nn -vvv -i lo port 50051 > tcp
```

#### 用户操作
创建用户  

`sudo useradd -d /home/postgres -m postgres`  
`sudo passwd postgres`

删除用户

`sudo userdel postgres`

登录用户

`su - postgres`

[更多参考](https://blog.csdn.net/beitiandijun/article/details/41678251)

[Linux如何查看所有的用户和组信息](https://www.cnblogs.com/selectztl/p/9523151.html)

#### 安装及卸载
```
sudo apt-get install <package>      // 安装
sudo apt-get update

apt-cache madison <package>         // 查看源 package 版本

# 删除软件及其配置文件
apt-get --purge remove <package>
# 删除没用的依赖包
apt-get autoremove <package>
# 此时dpkg的列表中有“rc”状态的软件包，可以执行如下命令做最后清理：
dpkg -l |grep ^rc|awk '{print $2}' |sudo xargs dpkg -P
# 当然如果要删除暂存的软件安装包，也可以再使用clean命令。
apt-get clean <package>
```

[Ubuntu apt-get彻底卸载软件包](https://www.jianshu.com/p/4a409053575a)

#### 文件重定向

举个例子：  
`nohup wget -m --ftp-user=wei --ftp-password=312 ftp://106.14.218.178/Percona-Share-Storage/ 1>ftp 2>&1 &`

1代表正常输出，2是错误输出。  
上面把所有的输出都重定向到 `ftp` 文件里。  
>`1>`、`2>&` 数字和符号之间没有空格。 

#### 别名
编辑 .bashrc，加入：
```
alias ll='ls -la'
```

#### 查看逻辑核数
```
cat /proc/cpuinfo| grep "processor"| wc -l
```

#### 查看端口号
```
sudo netstat -tunlp
```

#### 网络工具

`ip a` 查看网卡等  
`ethtool eth0` 查看某网卡带宽  
`nload lo` linux自带工具，查看某网卡实时流量  


##### iperf3
用于测试网络总带宽等。
```
# 服务端
iperf3 -s --port 50051
# 客户端
iperf3 -c 10.11.6.120 -b 0 -t 60 -i 5 -P 4 --port 50051
```
> -c 客户端ip，-b 比特率，0表示无限制，-t 总测试时间，-i 间隔多少秒输出信息，-P 线程数
