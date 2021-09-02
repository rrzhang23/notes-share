### 1. linux native aio 
POSIX AIO 是在用户控件模拟异步 IO 的功能，不需要内核支持，而 linux AIO 则是 linux 内核原声支持的异步 IO 调用，相关库文件为 libaio。

#### 接口、结构体等
libaio 实现的异步 IO 主要包含以下接口：

| <div style="width:100px">函数</div> | 功能 | 原型 |
|:--|:---|:---|
| io_setup | 创建一个异步IO上下文（io_context_t是一个句柄）| int io_setup(int maxevents, io_context_t *ctxp); |
| io_destroy | 销毁一个异步IO上下文（如果有正在进行的异步IO，取消并等待它们完成） | int io_destroy(io_context_t ctx); |
| io_submit | 提交异步IO请求 | long io_submit(aio_context_t ctx_id,  | long nr, struct iocb **iocbpp); |
| io_cancel | 取消一个异步IO请求 | long io_cancel(aio_context_t ctx_id, struct iocb *iocb, struct io_event *result); |
| io_getevents | 在指定时间内获取最少 min_nr 个，最多 nr 个完成的请求，timeout 为超时时间 | long io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct timespec *timeout); |

涉及到的数据结构主要有：  
| <div style="width:100px">数据结构</div> | 作用 |
| -- | :-- |
| io_context_t | AIO 上下文句柄，用来给一组异步 IO 请求提供一个上下文环境，每个进程可以有多个 aio_context_t，io_setup()  的第一个参数声明了同时驻留在内核中的异步 IO 上下文数量 |
| iocb | aio 回调结构，包含一次读写的 fd、内存区域的buf、读写数据量、偏移量等，同时还能携带一部分私有数据，后面 io_getevent() 函数调用后，可以从 io_event 里取出来。
| io_event | 包含 iocb 放进来的数据、iocb指针、读写结果 | 

#### 例子
```
一个大致流程如下：变量 io_ctx、io_cb、events 需要初始化
1. io_setup(num_events, io_ctx);        // 绑定一个上下文对应多少个请求/io_event
2. io_prep_pread(io_cb, fd, buf, len, offset);  // 设置 io_cb 的一些参数
3. io_cb->data = some_buf;              // 添加一些额外信息
4. io_submit(io_ctx, 1, &io_cb);        // 提交请求给内核
5. io_getevents(io_ctx, 1, 10, events, &timeout); // 获取请求结果

@ 代码段 1
// 定义一个读写单元，包含数据区和编号
struct Slot {
    int fd;
    byte *ptr;
    int slot_no;
    int len;
    off_t offset;
}

io_context_t io_ctx;    // IO 上下文
io_event events[10];         // 10 个 events
memset(&events[0], 0x0, sizeof(events[0]*10));
memset(&io_ctx,0,sizeof(io_ctx));

io_setup(10, io_ctx);   // 一个 aio 上下文绑定 10 个请求

struct iocb *io_cb;

// 写操作
Slot *slot = new Slot();
memset(&io_cb, 0, sizeof(*io_cb));
io_prep_pwrite(io_cb, file.fd, slot->ptr, slot->len, slot->offset);          // 将读写信息和 icob 绑定
iocb->data = slot;      // icob 携带的私有数据
io_submit(io_ctx, 1, &io_cb);    // 提交该 aio 请求
int num = io_getevents(io_ctx, 1, 10, events, &timeout);
// 若 num == 1,则上面的写成功

// 读操作
Slot *slot = new Slot();
memset(&io_cb, 0, sizeof(*io_cb));
io_prep_pread(io_cb, file.fd, slot->ptr, slot->len, slot->offset);          // 将读写信息和 icob 绑定
io_cb->data = slot;      // icob 携带的私有数据
io_submit(io_ctx, 1, &io_cb);    // 提交该 aio 请求
int num = io_getevents(io_ctx, 1, 10, events, &timeout);
// 若前面多次提交了 read 请求，则可以循环处理完成的读请求：
for 0 to num{
    iocb = reinterpret_cast<struct iocb *>(events[i].obj);
    Slot *slot = reinterpret_cast<Slot *>(iocb->data);
    more operation ...
}
```
> 参考：  
> [Linux下libaio的一个简单例子](https://www.cnblogs.com/3me-linux/p/4530245.html)  
> [linux AIO -- libaio 实现的异步 IO](https://www.cnblogs.com/cobbliu/articles/8487161.html)  
> [io_getevents](http://man7.org/linux/man-pages/man2/io_getevents.2.html)  


### 2. IO 子系统

主要涉及的文件：
```
storage/innobase/os/file.h file.cc                          // 页面压缩相关
storage/innobase/os/os0file.cc os/../include/os0file.h      // 文件读写，同步、异步IO、上层接口等
storage/innobase/os/os0event.cc os/../include/os0event.h    // 用于异步读写的共享信号量
storage/innobase/os/os0thread.cc os/..include/os0thread.h   // 对 thread 的封装，用于创建异步 IO 线程
storage/innobase/os/os0proc.cc os/..include/os0proc.h       // 进程相关
```

在 innobase IO 底层文件读写中， 将读写请求分为以下几类：  
* change buffer(ibuf)
* log
* read
* write 
* sync // 上面几个都是异步，这是个特殊

每一类都由一种专门的线程来处理请求（每中线程可以有多个，在 innodb中，ibuf、log线程只各有一个，read、write可以有多个），innobase 定义了一个 AIO（异步IO） 类来描述以上几种进程，os0file 代码里将每个 AIO 对象称为一个 aio array，代表该类线程读写的队列，队列由一个或多个段（segment）组成，每个段由 256 个 Slot(读写基本单元，描述一次 io 请求相关属性、内存缓冲区等) 组成。  
>段号解释：在IO系统里，上述几种类型现场是有一定顺序的，存在一个全局 seg，每个 aio array 里还有本地 seg。    

**AIO 属性包括**：Slot 队列、段数、表示队列空/满的信号量、还有一个锁来保护这些变量；在使用 linux native aio 的情况下，还定义了 native aio 上下文 io_context_t[]（长度为 local seg），io 事件结果 io_event[] (长度为 local seg * 256)。  

**AIO 函数包括** Slot、segment 的操作、上锁解锁、变量空间分配（create、start等）、调试用的打印。

**AIO、Slot、AIOHandler、LinuxAIOHandler、SimulatedAIOHandler之间的关系：**  
AIO 是一种异步读写线程 (aio array) 的读写队列，Slot 是队列里的组成单元；os0file.cc 提供了很多全局函数，同步 IO 可以直接调用，异步 IO 需要通过 AIOHandler 封装的静态函数来调用；最后两个类是 native aio 及
模拟异步 IO 的操作封装。

> 另外，os0file.cc 涉及到的其他数据结构 struct Slot、class AIO、strcut Block、class AIOHandler、class LinuxAIOHandler、SyncFileIO 在 [os0file 数据结构](http://note.youdao.com/noteshare?id=3c7885bc9f5c97328289d1326fb01b4d&sub=315320D2162B4C2EA76BA5384B312C63) 里有更多的代码注释。

#### 和其他模块的耦合点
AIO 及其他的几个数据结构是不会直接在其他模块的代码里调用的，MySQL 虽然用 C++ 重构了，但是仍然保留了一些 C 风格的特性，os0file 为上层提供的接口，并不是通过先声明对象，再通过对象调用自身函数这样的方式，class AIO 等数据结构对外部模块是不可见的。  
os0file.h 提供了系列以 `os_aio_`、`os_file_` 开头的函数接口供其他模块调用，`os_aio_` 提供 IO 线程的初始化、读写等功能、`os_file_` 提供文件`增删改`操作接口。

调用该 IO 模块的相关文件：
```
storage/innobase/srv/srv0start.cc   // 异步 IO 线程的初始化
storage/innobase/fil/fil0fil.cc     // 1. 其他模块想调用 IO 请求，先调该文件中的 fil_io(), fil_io() 再去调用 os0file.h 提供的 os_aio 接口；2. 创建 IO 线程时，也是先调该文件中 fil_io_wait()，再调 os0file.h 中对应 handler 线程函数。
```
1. IO 系统初始化及线程创建：
```
@ 代码段 2
// 系统初始化
srv0start::srv_start(n_readers, n_writers, n_slots_sync) {
    os0file::os_aio_init() {
        // 先创建各个 aio array
        AIO::start(256, n_readers, n_writers, n_slots_sync)) {
            s_reads  = create(LATCH_ID_OS_AIO_READ_MUTEX, n_readers * 256, n_readers) {
                // 在调用构造函数及 init() 为变量申请空间及初始化
                AIO *array = AIO::AIO();
                array->init();      // 初始化 slots、native aio 相关变量
                return array;
            }
            s_ibuf   = ...;
            s_log    = ...;
            s_writes = ...;
        }
    }
    
}
@ 代码段 3
// 线程创建
for(t from 0 to 总线程数) {
    IB_thread thread = = os_thread_create(线程类型, io_handler_thread, t);      // t 为全局段号
    thread.start();
}
io_handler_thread 是个函数：
srv0start::io_handler_thread() {
    while(...) {
        fil0fil::fil_aio_wait {
            os_aio_handler() {
                if (srv_use_native_aio) {
#elif defined(LINUX_NATIVE_AIO)
                    os_aio_linux_handler(segment, m1, m2, request) {
                        LinuxAIOHandler handler(global_segment);
                        handler.poll(m1, m2, request);
                    }
#endif
                } else {
                    os_aio_simulated_handler(segment, m1, m2, request);
                }
            }
        }
    }
}
```
代码段 3：  
层层往下分析，这里异步IO相当于生产者/消费者模型，io_handler_thread 通过一个无限循环来查询是否有请求完成，每进一次 while 相当于一次消费；  
而 LinuxAIOHandler::poll() 和 os_aio_simulated_handler()， 这两个函数提供了一个for(;;) ，检查后台是否有 slot->io_already_done（相当于生产者往队列里放进去东西），至少有一个 slot完成 IO ,则 io_handler_thread 一次消费成功。

2. 读写流程
os0file 提供的接口不会直接由 buffer 调用，fil0fil 文件提供了一层封装，buffer 是通过 fil0fil 里fil_io()->do_io()->os_aio() 调用读写请求的。  
os_aio 是一个宏，对应 os0file 里的 os_aio_func() 函数。该函数根据IO 请求模式判断是同步还是异步。
```
@ 代码段 4
os_aio_func() {
    // 同步读写函数调用栈 ：
    // os_file_read_func -> os_file_read_page -> os_file_pread -> os_file_io
    // 写同理，最终都是调用 os_file_io()
    // os_file_io 最终调用同步IO类 中 SyncFileIO::execute 执行的，实际上就是对库函数 pread、pwrite 做了很多封装。 
    if (aio_mode == AIO_mode::SYNC) {
        if (type.is_read()) { 
            return (os_file_read_func(type, file.m_file, buf, offset, n));
        } else if(type.is_write()) {
            return (os_file_write_func(type, name, file.m_file, buf, offset, n));
        }
    } else {
        // 根据是否使用 native aio, 调用 AIO::linux_dispatch 或 AIO::wake_simulated_handler_thread
        // 先分配 slot，再执行请求：
        slot = AIO::reserve_slot(); 
        array->linux_dispatch(slot);
        AIO::wake_simulated_handler_thread(AIO::get_segment_no_from_slot(array, slot));
    }
}
```
可以看出，请求类型是由上层调用线程指定的，os0file只是根据还类型判断，然后调用不同函数罢了。  

通常一个完整的读写请求都涉及到压缩/解压、加密/解密等过程，写请求先压缩加密，再写文件，最后进行文件打洞；读请求先读文件、再解压解密。  
同步调用栈：os_file_read_func/os_file_write_func -> os_file_io -> os_file_io_complete  
异步调用栈：先调 reserve_slot()，分配slot、压缩加密，再写文件，最后 AIOHandler::io_complete -> os_file_io_complete

> 压缩、加密、文件打洞参考：  
> [MySQL · 社区动态 · InnoDB Page Compression](http://mysql.taobao.org/monthly/2015/08/01/)  
> [linux 稀疏文件（Sparse File）](https://blog.csdn.net/cymm_liu/article/details/8760033)

### 3. 代码中 native aio 部分

回顾第一节 `1. linux native aio` 中的例子：
>一个大致流程如下：变量 io_ctx、io_cb、events 需要初始化
>1. io_setup(num_events, io_ctx);        // 绑定一个上下文对应多少个请求/io_event
>2. io_prep_pread(io_cb, fd, buf, len, offset);  // 设置 io_cb 的一些参数
>3. io_cb->data = some_buf;              // 添加一些额外信息
>4. io_submit(io_ctx, 1, &io_cb);        // 提交请求给内核
>5. io_getevents(io_ctx, 1, 10, events, &timeout); // 获取请求结果

// io_context_t、io_event、iocb 定义：
```
// 上下文 io_ctx、事件 events定义在 class AIO 中：
class AIO {
  ...
#if defined(LINUX_NATIVE_AIO)
  typedef std::vector<io_event> IOEvents;
  io_context_t *m_aio_ctx;      // 大小为 local seg 数
  IOEvents m_events;            // 大小为 256 * local seg
#endif 
  ...
}
// io_cb 定义在 struct Slot 中：
struct Slot {
  ...
#elif defined(LINUX_NATIVE_AIO)
  struct iocb control;
#else
  ...
}
```

// 调用流程
```
srv0start::srv_start() -> os0file::os_aio_init() -> AIO::start() -> AIO::create() -> AIO::init() -> AIO::init_linux_native_aio() -> linux_create_io_ctx() {
    // 设置一个上下文绑定多少 events
    io_setup(slots.size * local seg, m_aio_ctx);
}

os_aio_func() {
    AIO::reserve_slot() {
        // 先从队列里取一个slot，并为之设置相关属性
        struct iocb *iocb = &slot->control;
        // 设置 iocb 属性
        if (type.is_read()) {
          io_prep_pread(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
        } else {
          io_prep_pwrite(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
        }
        iocb->data = slot;
    }
    linux_dispatch(slot) {
        // 提交 IO 请求
        io_submit(m_aio_ctx[io_ctx_index], 1, &iocb);
    }
}

LinuxAIOHandler::collect() {
    ret = io_getevents(io_ctx, 1, m_n_slots, events, &timeout);
    for( i from 0 to ret) {
        iocb = reinterpret_cast<struct iocb *>(events[i].obj);
        Slot *slot = reinterpret_cast<Slot *>(iocb->data);
        ... // 处理完成 IO 请求后的 slot
    }
}
```


**poll() 和 collect()**   
os_aio_func 相当于一个生产者，每调用一次就把读写请求放入 aio array 队列，而 innobase 起的几个异步 IO 线程 io_handler_thread 相当于消费者（代码段 3 描述了消费者线程）。  
poll 函数提供了了 for(;;) 死循环检查线程内 aio array 是否有 slot->io_already_done，有的话则进行相关操作；若没有，则需要询问内核，由 collect 完成。  
 collect() 每隔 0.5秒 调用一次 io_getevents 向内核查询是否有 io 完成，若有则修改 slot 相关属性，并调用 io_complete 完成相关解压、解密操作，poll 函数下次进入 for 语句就能看到完成的 slot。collect() 可以理解为生产者。

### 4. 模拟 aio
#### 和 native aio 比较

涉及的变量及函数：
```
os_aio_segment_wait_events[gobal segment no.]   // 队列信号量，大小为全局段的数量
wake_simulated_handler_thread()                 // 将对应段的信号量设置为 true，表明该段内有读写请求
os_aio_simulated_handler()                      // 作用类似 LinuxAIOHandler::poll()，等待至少一个 io 完成请求
```

SimulatedAIOHandler 没有内核提供的 io_events 来查询哪些 IO 请求已经完成，但是其通过自定的信号量 `os_event_t *os_aio_segment_wait_events`(数组大小为 global segment) 作为队列生产、消费的沟通。

先看"生产部分":  
有 IO 请求过来时，os_aio_func() 函数判断是否使用了 native IO，若使用了，则调用 linux_dispatch 将请求交给内核；若使用的是模拟异步IO，则调用 wake_simulated_handler_thread 将该请求所在 slot 的全局段对应的 os_aio_segment_wait_events[seg number] 设置为true，表明该段内有读写请求，实际的io 动作在 os_aio_simulated_handler 完成。

"消费者"os_aio_simulated_handler：  
全局函数 os_aio_simulated_handler 完成了类似 LinuxAIOHandler::poll() 的功能。逻辑和 poll() 类似，做了一些类似组提交的优化，多个连续的读请求 Slot 会被放到一起，只做一次 pread。



> 更多请参考[MySQL · 引擎特性 · InnoDB 文件系统之IO系统和内存管理](http://mysql.taobao.org/monthly/2016/02/02/)




