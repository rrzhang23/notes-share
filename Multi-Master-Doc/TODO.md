1. poll（完成）
2. open 函数哪里调用（完成。创建日志等的函数入口，主要找文件名称在哪里。在srv0start::srv_start()函数）
3. 调研 polarfs 论文接口部分（完成）
4. MySQL 文件 IO 总结中的 TODO。（完成）
5. 日志代码阅读，日志号怎么分配 
6. os0file 调用的库函数接口整理出来（完成）
7. mini 事务
8. srv0start，innobase的初始化、handler/ha_innodb.cc
9. 




[INNODB记录格式](https://blog.csdn.net/bohu83/article/details/81226565)讲了抛开innobase，server层的格式row数据怎样存到储存引擎的。

[MySQL InnoDB存储引擎启动过程源码分析](https://baijiahao.baidu.com/s?id=1644270736941739246&wfr=spider&for=pc)

[Innodb 插件初始化](https://blog.csdn.net/innobase/article/details/53306934)

> #mysql_declare_plugin 中 innodb_init 参数怎么传的