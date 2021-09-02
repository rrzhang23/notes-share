## rpm 与 yum
rpm 是 Redhat 系统的包管理方式，可处理 .rpm 软件包之类的依赖，.rpm是不会暴露源码的软件包。  
yum 则是一层嵌套，可在线拉取 rpm 等。

## rpm
安装
~~~cpp
// -i install, -v 提供冗长信息（verbose）,-h 哈希
rpm -ivh *.rpm
~~~

卸载
~~~cpp
rpm -e packgename
rpm -e --nodeps packgename
~~~

查询所有安装的包： 
~~~cpp
rpm -qa
~~~

查询某个包：
~~~cpp
rpm -qa | grep xxx
rpm -qi xxx         // 查询某个包的安装信息
~~~

查询某个文件是哪个rpm包产生：
~~~cpp
rpm -qf /etc/yum.conf
~~~

查询软件的安装路径：
~~~cpp
rpm -ql xxx
~~~

升级
~~~cpp
rpm -Uvh xxx
~~~

## yum
~~~cpp
# yum install xxx       // 安装xxx软件
# yum info xxx          // 查看xxx软件的信息
# yum remove xxx        // 删除软件包
# yum list xxx             // 列出软件包
# yum clean             // 清除缓冲和旧的包
# yum provides xxx      // 以xxx为关键字搜索包（提供的信息为关键字）
# yum search xxx        // 搜索软件包（以名字为关键字）,在rpm包名,包描述等中搜索
# yum groupupdate xxx
# yum grouplist xxx
# yum groupremove xxx   // 这三个都是一组为单位进行升级 列表和删除的操作。。比如 "Mysql Database"就是一个组会同时操作相关的所有软件包；
~~~

~~~cpp
// 先找出安装的id,再 undo，比如 ncurses
# yum history list ncurses
Loaded plugins: fastestmirror, langpacks, nvidia, product-id, search-disabled-repos, subscription-manager

This system is not registered with an entitlement server. You can use subscription-manager to register.

ID     | Login user               | Date and time    | Action(s)      | Altered
-------------------------------------------------------------------------------
   105 |  <zhangrongrong>         | 2021-06-17 08:31 | Install        |    1 PP
     1 | System <unset>           | 2020-10-12 11:37 | Install        | 1306   
history list


# sudo yum undo 105
~~~



[参考](https://zhuanlan.zhihu.com/p/27724520)