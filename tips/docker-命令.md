hub.docker.com 查看库的多版本

```
docker images                       // 显示已安装的镜像
docker pull centos:centos7.4.1708   // 拉取版本 7.4.1708 的centos
docker rmi centos:centos7.4.1708    // 删除镜像

docker run [OPTIONS] REPOSITORY:TAG [COMMAND] [ARG...]
docker run -itd --name centos7-rrrzhang centos:centos7.4.1708 /bin/bash     // 打开 IMAGE 为 centos:centos7.4.1708 的镜像，[OPTIONS] 为 -itd --name centos7-rrrzhang，/bin/bash 是 [COMMAND]

docker ps [-a]                      // 显示进程，-a 是显示所有进程，包括已经退出的
docker stop 'CONTAINER ID'
docker rm 'CONTAINER ID'            // stop 后，重启这个镜像会有问题，需要彻底 remove

docker commit [OPTIONS] CONTAINER [REPOSITORY[:TAG]]
docker commit -a "rrzhang" -m "notes" -p e58a0d765158 centos:centos7.4-v1    // -a 作者，-m 解释，-p 暂停当前容器

docker search postgresql            // 搜索镜像
docker stats                        // 查看各容器的状态
```

##### docker centos 相关
```
[docker] docker run -d --name centos7-rrrzhang --privileged=true -p 6119:22 centos:centos7.4.1708 /usr/sbin/init

[docker] docker attach e58a0d765158          // attach 到 centos7-rrrzhang，e58a0d765158 是 CONTAINER ID

# 开始是没有 sudo, vim, ifconfig, ssh 的，需要安装
[root@11f76ac580c3 /]# cat /etc/redhat-release
CentOS Linux release 7.7.1908 (Core)
// 修改 root 密码 123456
[root@11f76ac580c3 /]# passwd
[root@11f76ac580c3 /]# yum -y install sudo
[root@11f76ac580c3 /]# sudo yum -y install openssh-server openssh-clients vim
[root@11f76ac580c3 /]# sudo yum -y install gcc gcc-c++ kernel-devel
[root@11f76ac580c3 /]# vim /etc/ssh/sshd_config

PubkeyAuthentication yes    #启用公钥私钥配对认证方式
AuthorizedKeysFile .ssh/authorized_keys #公钥文件路径（和上面生成的文件同）
PermitRootLogin yes         #root能使用ssh

[root@11f76ac580c3 /]# /bin/systemctl restart sshd.service

实际上 systemctl 会报如下错：
'Failed to get D-Bus connection: Operation not permitted'
参考 [Docker容器使用问题](https://blog.csdn.net/zhenliang8/article/details/78330658)

先 commit 容器至某个镜像，再如下重启：
docker commit -a "rrzhang" -m "add sudo, vim, ifconfig, ssh, and modify /etc/ssh/sshd_config" -p 88eb17422788 centos:centos7.4-v1
docker run -d --name centos7-rrrzhang --privileged=true -p 6119:22 centos:centos7.4-v1 /usr/sbin/init
docker exec -it centos7-rrrzhang /bin/bash

[root@11f76ac580c3 /]# exit

# 之后就可以用 ssh 登录 centos
ssh root@127.0.0.1 -p6119
```

```

```