## 说明
* **作用**：MySQL X是一个插件，5.7.12版本开始提供安装，8.0 默认集成在 MySQL 里，它为 MySQL 提供 NoSQL JSON 文档存储。
* **完整 X 协议流程图**： 
![X 协议概述](https://dev.mysql.com/doc/dev/mysql-server/latest/inline_umlgraph_46.png)
* X plugin 分为三个部分：X DevAPI、X Protocol、X Plugin。
  * X DevAPI，可以提供结构化查询之外的调用方式，有 Java、Python、.Net、C++ 等系列主流开发语言调用接口。
  * X Protocol，这是 MySQL 自己新建立的用于 X 的协议，基于Google Protobuffers，个人理解就是 MySQL 对其做了层封装，所以才能支持多语言的调用。服务端的这个插件在收到请求之后，会将其翻译为对应的SQL语句，然后转交给MySQL主服务执行，并将结果集返回给客户端。
  * X Plugin，MySQL服务器与客户端之间的实际接口。
  * 一个完整的协议过程如上图所示，用户调用client端提供的接口，X Protocol 将其编码为 protoc 协议格式，发送给 server 端，server 端解码出来之后翻译成SQL语句并执行。  
  client 和 server 使用通信tcp协议。

## 源码
X Plugin关于 libevent 代码在包 ngs下：
+ mysql-server-8.0\plugin\x\ngs\src\socket_events.cc

最终调用的 mysql-server-8.0\extra\libevent\event.c 下的相关函数实现的。

## 参考
[Understanding MySQL X (All Flavors)](https://www.percona.com/blog/2019/01/07/understanding-mysql-x-all-flavors/)  
[X协议](https://dev.mysql.com/doc/dev/mysql-server/latest/mysqlx_protocol.html)  
[MySQL X协议分析](https://www.jianshu.com/p/b91f2a6311e9)  


