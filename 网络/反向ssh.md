> 参考:  
> [搭建ssh反向隧道](https://www.jianshu.com/p/b1cc3b5aa00d)  
> [利用SSH反向隧道，连接内网服务器](https://www.cnblogs.com/felixnet/p/7767191.html)

现有两台机器，实验室机房内网 `zhangrongrong@10.11.1.193`，阿里云公网 `rrzhang@47.102.100.148`，利用 ssh 和 autossh 实现反向代理，从外网访问机房机器。

#### 1、编辑 ssh 配置文件，确保 `GatewayPorts yes` 打开，并重启 sshd 服务。
```
47.102.100.148:
sudo vim /etc/ssh/sshd_config
sudo systemctl restart sshd
```

#### 2、
内网主机配置可以有两种选项: ssh 和 autossh，区别在于 ssh 容易超时断开，autossh 可以自动重连。

2.1、ssh
```
10.11.1.193：
ssh -NfR 1193:localhost:22 rrzhang@47.102.100.148 -p 2222

rrzhang 是外网机器用户名，47...是外网机器ip，2222 是外网机器 ssh 端口。访问 47.102.100.148:1193 即能访问到 10.11.1.193:22

-C 压缩数据传输
-f 将 ssh 转到后台运行，即认证之后，ssh 自动以后台运行。不在输出信息
-n 将 stdio 重定向到 /dev/null，与 -f 配合使用
-N 不执行脚本或命令，即通知 sshd 不运行设定的 shell 通常与 -f 连用
-T 不分配 TTY 只做代理用
-q 安静模式，不输出 错误/警告 信息
```

通过查看外网端口情况、ssh登录验证：
```
47.102.100.148:
sudo netstat -tunlp
```
![](https://note.youdao.com/yws/api/personal/file/WEB0bb76af3fbb9bec948b0d4d427cf3658?method=getImage&version=16765&cstk=w734ld5I)

内网执行那个 ssh 命令后，会在 rrzhang 用户下起一个端口 2222 的监听进程。

```
47.102.100.148:
ssh zhangrongrong@47.102.100.148 -p 1193
或
ssh zhangrongrong@localhost -p 1193
```
**尝试登录，zhangrongrong 是内网用户名，ip是外网ip，端口是新建的端口。**  

这里并不需要像[linux 修改 ssh 默认端口 22](http://note.youdao.com/noteshare?id=88dafabed1e13a567933e1dae3263251)那样，配置 `/etc/ssh/sshd_config` 文件来分配端口，因为该端口已经被 rrzhang 用户启用了。
> 阿里云可能需要配置防火墙，打开 2222 端口。参考 [linux 防火墙](http://note.youdao.com/noteshare?id=59c28b9c9beed254de6b17f7ebe1c1ef)。  
> 另外还需要在阿里云控制台打开端口。
 

2.2、autossh

下载安装，可以通过 `yum` 或 `apt-get` 来安装，也可以下载 bin 包，自己配置 PATH 路径。

把 `~/.ssh/id_rsa.pub` 内容发送到外网 `authorized_keys` 里：
```
10.11.1.193：
ssh-copy-id -i /home/zhangrongrong/.ssh/id_rsa.pub "-p 2222 root@47.103.213.62"
~/.local/autossh-1.4g-1-x86_64/bin/autossh -M 11930 -f -NR 1193:localhost:22 root@47.103.213.62 -p 2222

-M	外网主机接收内网主机信息的端口，超时断开之后，内网需要通过5555主动建立连接。
```
后面可以跟 2.1 一样去验证。