# MySQL 文件 IO 总结

## 1. 文件物理结构

**InnoDB逻辑存储空间** 称为表空间，层次分为段（segment）、区（extent）、页（page）页。  

**表空间**：一个逻辑概念，正常情况下，可以认为每个表空间在储存上都对应一个物理文件。大概分为以下几种文件：  
日志文件、主系统表空间文件ibdata、undo tablespace 文件、临时表空间文件、用户表空间文件。  
* ***日志文件*** 主要用于记录redo log，默认情况下，日志是以512字节的block单位写入。由于现代文件系统的 block size 通常设置到4k，InnoDB提供了一个选项，可以让用户将写入的redo日志填充到4KB，以避免 read-modify-write 的现象。

* ***ibdata*** 是InnoDB最重要的系统表空间文件，它记录了InnoDB的核心信息，包括事务系统信息、元数据信息，记录InnoDB change buffer的btree，防止数据损坏的double write buffer等等关键信息。

* ***undo*** 独立表空间是一个可选项，通常默认情况下，undo数据是存储在ibdata中的，但你也可以通过配置选项 innodb_undo_tablespaces 来将undo 回滚段分配到不同的文件中。

* ***临时表空间*** MySQL 5.7 新开辟了一个临时表空间，默认的磁盘文件命名为ibtmp1，所有非压缩的临时表都存储在该表空间中。由于临时表的本身属性，该文件在重启时会重新创建。

* ***用户表空间*** 用于自己创建的表空间，通常分为两类，一类是一个表空间一个文件，另外一种则是5.7版本引入的所谓General Tablespace，在满足一定约束条件下，可以将多个表创建到同一个文件中。

#### 段、区、页、行之间关系：
**行**：InnoDB存储引擎是面向行的，数据按行进行存放。  
**页**：页是InnoDB磁盘管理的最小单位，页的大小是16KB。  
**区**：由64个连续的页组成，每个页大小为16KB，则一个区为1MB。     
**段**：常见的段有数据段、索引段、回滚段。  


>TODO:
>1. 跑通 MySQL ,看磁盘上到底储存了哪些文件类型。
>  
>更多请参考：
>* [InnoDB表存储结构及keyring加密](https://www.cnblogs.com/smartwhite/p/7760318.html)  
>* [MySQL · 引擎特性 · InnoDB 文件系统之文件物理结构](http://mysql.taobao.org/monthly/2016/02/01/)
>
>
>

## 2. 文件IO概要
**源代码文件**：  
和文件 IO 相关代码都放在 storage/innobase/os 目录下，主要涉及：  
```
os/file.h file.cc  // 页面压缩相关
os/os0file.cc os/../include/os0file.h  // 文件读写，同步、异步IO、上层接口等
os/os0event.cc os/../include/os0event.h //  
os/os0thread.cc os/..include/os0thread.h // 异步IO线程
os/os0proc.cc os/..include/os0proc.h // 进程相关
```
以及
```
storage/innobase/srv/srv0start.cc // 负责异步IO线程的初始化
storage/innobase/fil/fil0fil.cc // 调用 os_aio_handle 最为频繁的文件
```
> TODO: 
> 1. 文件锁的管理相关代码阅读
> 2. os/os0event.cc、os/os0proc.cc  
>
> reply 1: 这里没有并发控制，并发控制在 buffer 层，AIO 是有锁，那只是保护共享变量的。  
> reply 2: event 是供 AIO 使用的信号量，异步 IO 需要知道队列的情况（满、空等）。

### InnoDB IO子系统
#### 同步异步背景
关于同步和异步区别可以参考这篇文章(网上类似文章有很多)：[同步IO、异步IO的区别](https://www.cnblogs.com/zhangmingda/p/9396994.html)。  
同步和异步的概念不仅仅存在于IO之中，其他地方也有很多应用，但原理都几乎相同。简单总结同步和异步的区别就是：  
***同步需要等待（阻塞），异步靠通知（消息）。***

#### 同步IO及文件DDL操作：  
接口在 `os/os0file.cc os/../include/os0file.h` 里提供。

同步读写采用 `pread/pwrite` or `lseek+read/write`方案，定义在函数 `os_file_read_func、os_file_write_func` 里。  

DDL相关接口：  
```
os_file_create_tmpfile // 创建临时文件
os_file_lock // 对文件加进程锁
os_file_create_simple // 创建或者打开一个文件
os_file_create //  创建或者打开一个文件，如果操作失败会重试，直到成功
os_file_close // 关闭打开的文件
os_file_get_size // 获得文件的大小
os_file_set_size // 设置文件的大小并以0填充文件内容
os_file_flush // 将写的内容fsync到磁盘
os_file_read // 从文件中读取数据
os_file_write 
other_DDL_function...
```
#### 异步 IO
涉及到的主要数据结构： `class AIO`、`struct Slot`、`class LinuxAIOHandler` `class SimulatedAIOHandler`。  
* ***class AIO***：一个 AIO 对象对应一种（注意不是一个，读写线程可能分别有多个） IO 线程，每种线程都有一个读写队列 array，这个在类内定义为 `std::vector<Slot> m_slots`，这里Slot代表一个异步IO上下文。  
* ***struct Slot***：异步IO上下文，记录了在读写队列里的位置、读写内容、IO请求类型、在对应文件内的offset、文件句柄等信息。
* ***class LinuxAIOHandler***：Linux native AIO 线程句柄，包含当前线程的段位置、属于哪个AIO、有多少slots等信息。同时还有`poll`、`resubmit`、`check_state`、`find_completed_slot`等系列线程函数处理IO请求。
* ***class SimulatedAIOHandler***：类似 LinuxAIOHandler，在linux提供原生异步IO包 `libaio` 之前，MySQL 通过模拟达到异步IO效果。

**异步IO一些内存结构初始化**  
前面讲每个AIO对象都绑定了一种IO线程，innobase一共有四种IO线程，分别处理：`ibuf`（change buffer）、`log`、`read`、`write`，其中 ibuf 和 log每种只有一个线程来处理，read 和 write 可以有多个线程，但是仍然分别只有一个AIO对象，通过segment来划分Slot段。  
可以这么理解，在innobase里 `static AIO *s_reads` 对象始终是只有一个的，每个对象拥有一个Slot IO队列，队列长度始终是256的倍数（每个线程最多处理这么长的队列）。通过段号来区分各自线程对应队列的读写位置。假设分别有2个read、write线程，段号 `0` 对应ibuf，`1` 对应log队列，`2、3`对应读队列，`4、5`对应写队列。  

**源码分析**  
在 innobase/include/svr0svr.h 里定义了几个相关变量：
```  
extern ulint srv_n_file_io_threads; // 异步IO线程总数
extern ulong srv_n_read_io_threads; // 读线程数
extern ulong srv_n_write_io_threads; // 写线程数
```

1. AIO 类初始化时，对应源码及调用栈：
```
srv0start::srv_start(){
    os0file::os_aio_init(srv_n_read_io_threads, srv_n_write_io_threads, SRV_MAX_N_PENDING_SYNC_IOS){ // SRV_MAX_N_PENDING_SYNC_IOS宏定义为 100 
        AIO::start(limit, n_readers, n_writers, 100)){
            AIO::s_read   = create(LATCH_ID_OS_AIO_READ_MUTEX, n_readers * n_per_seg, n_readers);
            AIO::s_ibuf   = create(LATCH_ID_OS_AIO_IBUF_MUTEX, n_per_seg, 1);
            AIO::s_log    = create(LATCH_ID_OS_AIO_LOG_MUTEX, n_per_seg, 1);
            AIO::s_writes = create(LATCH_ID_OS_AIO_WRITE_MUTEX, n_writers * n_per_seg, n_writers);
        }
    }
}
```
AIO::create() 会根据线程的数量来初始化 `AIO::m_slots` 的大小，2个read线程的 s_read 就会拥有512长度的 m_slots，不同线程根据段号来分别读 0-255、256-511的slot。

> 参考：[Mysql Innodb中的Linux native异步I/O(一) 内存结构的初始化](https://www.jianshu.com/p/dd7e1e560af0)
2. 相关线程初始化时，对应源码及调用栈：
```
srv0start::srv_start(){
    // 初始 srv_n_file_io_threads为 0
    srv_n_file_io_threads = srv_n_read_io_threads;
    srv_n_file_io_threads += srv_n_write_io_threads;
    if (!srv_read_only_mode) {
    // 只读模式不需要 log 和 ibuf，非只读模式需要添加2个线程数
    srv_n_file_io_threads += 2;
  } 
  // 初始化每个异步IO线程，t 就是段号segment，初始化时就已经绑定了
  for (ulint t = 0; t < srv_n_file_io_threads; ++t) {
    IB_thread thread; // 定义在 os0thread.cc 中
    if (t < start) {
      if (t == 0) {
        thread = os_thread_create(io_ibuf_thread_key, io_handler_thread, t);
      } else {
        thread = os_thread_create(io_log_thread_key, io_handler_thread, t);
      }
    } else if (t >= start && t < (start + srv_n_read_io_threads)) {
      thread = os_thread_create(io_read_thread_key, io_handler_thread, t);
    } else if (t >= (start + srv_n_read_io_threads) &&
               t < (start + srv_n_read_io_threads + srv_n_write_io_threads)) {
      thread = os_thread_create(io_write_thread_key, io_handler_thread, t);
    } else {
      thread = os_thread_create(io_handler_thread_key, io_handler_thread, t);
    }
    thread.start();
  }
}

io_handler_thread_key，暂时没啥用（和调试相关）

os_thread_create是个宏，定义在 os0thread-create.h 中，对应函数为：
IB_thread create_detached_thread(args...);

io_handler_thread，回调函数，srv0start里定义如下，segment 就是上面传入的段号 t：
static void io_handler_thread(ulint segment) {
  while (srv_shutdown_state.load() != SRV_SHUTDOWN_EXIT_THREADS ||
         buf_flush_page_cleaner_is_active() || !os_aio_all_slots_free()) {
    fil_aio_wait(segment);
  }
}
可以看到真正的回调函数是 fil0fil.cc 里定义的 `fil_aio_wait()`。

再扒一扒 fil_aio_wait():
fil0fil::fil_aio_wait(segment){
    os0file::os_aio_handler(segment, ...){
        if (srv_use_native_aio) {
            os_aio_linux_handler(segment, ...);
        } else {
            os_aio_simulated_handler(segment, ...);
        }
    }
}
函数判断了是否使用了系统的 native AIO，再具体调用 os_aio_linux_handler，还是模拟的 os_aio_simulated_handler。 

```


> TODO：
> 1. SimulatedAIOHandler实现阅读
> 
> reply 1: 完成，见[IO 代码详细总结](http://note.youdao.com/noteshare?id=4078e899c3b81d9abcbe6132e0ee41ba&sub=05DD3C29DFB04B66A2DFD17A71DDF4F1)



>更多参考：
>* [MySQL · 引擎特性 · InnoDB IO子系统](http://mysql.taobao.org/monthly/2017/03/01/)
>* [MySQL · 引擎特性 · InnoDB 文件系统之IO系统和内存管理](http://mysql.taobao.org/monthly/2016/02/02/)
>* [MySQL系列：innodb源码分析之文件IO](https://blog.csdn.net/yuanrxdu/article/details/41418421)

## 文件IO流程图
![innobase 文件IO调用流程图](https://note.youdao.com/yws/api/personal/file/934E9854AABC4BDF95C5824A0D5FC10B?method=download&shareKey=2feacba3b514795bd8aa0738df462b7d)
