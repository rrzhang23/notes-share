
1、修改 ssh 配置文件，加入 `Port=2222`
```
sudo vim /etc/ssh/sshd_config
```

2、查看端口情况
```
sudo netstat -tunlp
```

![netstat -tunlp](图片/netstat%20-tunlp.png)

端口 22 和 2222 具有同一个进程 id，且都属于系统进程，不属于任何用户进程。

这时可以用 `ssh user@ipaddress -p 2222` 来登录机器。