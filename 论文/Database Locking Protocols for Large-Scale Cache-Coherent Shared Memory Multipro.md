### 标题
Database Locking Protocols for Large-Scale Cache-Coherent Shared Memory Multiprocessors Design, Implementation and Performance

shared memory database(SM DB) 

研究了两种在缓存一致的共享内存多处理器数据库系统中实现锁定的方法。第一种方法是 `shared-memory locking(SML)`，它允许多处理器的每个节点（处理器）直接通过使用高速缓存一致性共享内存来获取和释放锁;第二种方法是 `message-passing locking(MPL)`，通常要求将消息发送到位于远程节点上的锁定管理器。

### 2.1 锁介绍
IX、IS意向锁，热锁相关概念
### 2.2 KSR-1 多核处理器
实现缓存一致性方案的硬件元素包括`缓存控制器`，`缓存目录`和`缓存本身`。高速`缓存`包含高速缓存的数据，而高速`缓存目录`包含所有高速缓存的数据的地址。

