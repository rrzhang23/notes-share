1. polarfs 接口 libpfs。
> Second, PolarFS provides a POSIX-like ﬁle system API, which is intended to be compiled into the database process and replace the ﬁle system interfaces provided by operating system, so that the whole I/O path can be kept in user space.（第一节）

> libpfs is a user space ﬁle system implementation library with a set of POSIX-like ﬁle system API.（第三节第二段）

文章两次提到计算层和存储层的接口相当于 POSIX IO 接口，说白了就是用自己的函数替换了库函数。  
相关接口如下：  
![libpfs interfaces](https://note.youdao.com/yws/api/personal/file/C8448A147EE34994A91DDB0E052E6424?method=download&shareKey=980cabdc58f47630d4f270ce9de337d1)


2. libpfs 和 PolarSwitch  
一个 libpfs 进程对应一个计算节点，作为计算和存储的接口，它将文件读写的偏移量转换成底层块的偏移量，交给 PolarSwitch 做读写。