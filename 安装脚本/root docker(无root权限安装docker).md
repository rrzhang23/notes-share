官方文档：[Run the Docker daemon as a non-root user (Rootless mode)](https://docs.docker.com/engine/security/rootless/)
[下载目录](https://download.docker.com/linux/static)

## 前置条件

需要系统有这两个可执行文件:`newuidmap`、`newgidmap`。

然后写入 `/etc/subuid`、`/etc/subgid`:
~~~cpp
echo "zhangrongrong:100000:65536" | sudo tee /etc/subuid
echo "zhangrongrong:100000:65536" | sudo tee /etc/subgid
~~~

参考: [容器安全拾遗 - Rootless Container初探](https://developer.aliyun.com/article/700923)

接着写入 `/etc/sysctl.conf`：
~~~cpp
# for docker :
user.max_user_namespaces=28633
~~~

~~~cpp
wget https://get.docker.com/rootless
mv rootless rootless.sh
chmod 775 rootless.sh
~~~








