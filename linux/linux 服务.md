> 参考  
> [利用systemctl管理Tomcat启动、停止、重启及开机启动详解](https://www.jb51.net/article/126948.htm)  
> [Linux 服务管理两种方式service和systemctl](https://www.cnblogs.com/shijingjing07/p/9301590.html)  
> [Systemd 指令 ](https://www.cnblogs.com/zwcry/p/9602756.html)

```
# 刷新
$ sudo systemctl daemon-reload

# 启动tomcat
$ systemctl start tomcat

# 重启tomcat
$ systemctl restart tomcat

# 停止tomcat
$ systemctl stop tomcat


# 服务加入开机启动
$ systemctl enable tomcat

# 禁止开机启动
$ systemctl disable tomcat

# 查看状态
$ systemctl status tomcat
```