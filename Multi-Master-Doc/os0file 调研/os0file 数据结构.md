#### Block
压缩和加密用到的临时块，系统维护一个全局 block 数组，大小为128，当申请 block 时，若数组比较繁忙，则申请一个临时 temp block。  
`m_in_use` 指示数组内该 block 是否在使用， `pad` 貌似用于内存对齐。
```
struct Block {
  Block() : m_ptr(), m_in_use() {}
  byte *m_ptr;
  byte pad[INNOBASE_CACHE_LINE_SIZE - sizeof(ulint)];
  lock_word_t m_in_use;
};
// 涉及到的函数
os_alloc_block();       // 分配的空间，前面用于 block 结构体，后面就是 ptr 指向的空间
os_free_block();    
os_file_compress_page();
os_file_encrypt_page();
os_file_encrypt_log();
```

---

#### Slot
每个 aio array 都存在一个 Slot 数组，作为 IO 基本单元。  
当读写线程在为一个 IO 请求调用 AIO::reserve_slot 分配 Slot 时，可能会涉及加密、压缩，造成文件空洞。  
关于文件打洞，压缩后，会在原来的读写长度上留下一段空白的空间，需要进行打洞操作，但是磁盘上又不存储这些空洞内容，节省空间。
```
struct Slot {
  bool is_reserved{false};          // 表示被分配出去，在 reserve_slot 里设置为 true
  ib_time_monotonic_t reservation_time{0};  // 时间，模拟异步IO 用得到
  byte *buf{nullptr};               // IO 读写数据开始 
  byte *ptr{nullptr};               // 和 buf指向同一块空间的不同位置，IO 读写到的位置，不一定一次能读写完
  IORequest type{IORequest::UNSET}; // 
  os_offset_t offset{0};            // 文件中的对应偏移量
  pfs_os_file_t file;               // fd

  const char *name{nullptr};        // 文件名称/目录
  bool io_already_done{false};      // 标志 IO 是否完成 
  fil_node_t *m1{nullptr};          // 
  void *m2{nullptr};                // 

#if defined(LINUX_NATIVE_AIO)
  struct iocb control;              // 
  int ret{0};                       // 返回值
#else
  ulint len{0};                     // 压缩、加密后的长度
  ulint n_bytes{0};                 // 已经读写完的数据量
  uint32 original_len{0};           // 压缩、加密前的数据长度
  Block *buf_block{nullptr};        // 压缩、加密后的 IO 块
  bool skip_punch_hole{false};      // 
  void *encrypt_log_buf{nullptr};   // 加密 log 用到的 buf

  Slot() {  // 初始化时就分配了 control 空间
#if defined(LINUX_NATIVE_AIO)
    memset(&control, 0, sizeof(control));
#endif /* LINUX_NATIVE_AIO */
  }
}
```
相关调用：
```
// 初始化
AIO::create() {
    AIO::AIO(): m_slots(n) { // 分配空间
    array->init(){
        init_slots() { }
    }
}
// slot 使用
os_aio_func(){
    // 进入异步 IO 后，
    slot = array->reserve_slot();
}
// slot 释放
release(Slot *slot);
```

> TODO：
> 1. m1 和 m2 变量干啥的。
>
>

---

#### AIO
```
class AIO {
// 变量：
  mutable SysMutex m_mutex;
  typedef std::vector<Slot> Slots;
  Slots m_slots;            // aio array 由多个 slots 构成
  ulint m_n_segments;       // 一个 aio array 分为多个段，每个线程处理一个段，互不干扰
  os_event_t m_not_full;    // 通知该 array 未满，event定义在 os0event.cc 中
  os_event_t m_is_empty;    // 通知该 array 是空的
  ulint m_n_reserved;       // 该 array 已分配出去的 Slot 数

#if defined(LINUX_NATIVE_AIO)  
  io_context_t *m_aio_ctx;  // 每段一个，io_context_t 是 libaio 里的结构， completion queue for IO. There is one such queue per segment. Each thread will work on one ctx exclusively.
  typedef std::vector<io_event> IOEvents;   // The array to collect completed IOs. There is one such event for each possible pending IO. The size of the array is equal to m_slots.size().
  IOEvents m_events;
#endif /* LINUX_NATIV_AIO */

// 几种不同的 aio array
  static AIO *s_ibuf;
  static AIO *s_log;
  static AIO *s_reads;
  static AIO *s_writes;
  static AIO *s_sync;
  
// 函数
    init();
    reserve_slot();         // 获取一个 Slot
    pending_io_count();     // 返回已经分配的 Slot
    release(Slot *slot);    // 释放某 Slot，在已获取锁的情况下，直接置is_reseverd = false,不用其他free操作
    release_with_mutex();   // 释放某 Slot
    slots_per_segment();    // 每个seg有多少 Slot
    get_n_segments();       // 该array 内seg数
    
    is_mutex_owned();       // 是否持有锁
    acquire();              // 上锁
    release();              // 解锁
    
#ifdef LINUX_NATIVE_AIO    
    linux_dispatch();       // IO请求分发给kernel层
    io_events();
    io_ctx();
    linux_create_io_ctx();
    is_linux_native_aio_supported();
    init_linux_native_aio();
#endif
    create();
    start();
    shutdown();                     // 释放该aio array
    get_array_and_local_segment();  // 给定全局 seg,返回 aio array和local seg
    select_slot_array();            // 根据给定IORequest、AIO_mode类型，返回对应 aio array
    get_segment_no_from_slot();     // 计算全局 seg
    wake_simulated_handler_thread(ulint global_segment);
    wake_simulated_handler_thread(ulint global_segment, ulint segment);
    is_read();                      // 是否是 s_read aio array   
    wait_until_no_pending_writes(); // 写队列为空
    total_pending_io_count();       // 所有类型array分配的 Slot 数
    init_slots();
    print_segment_info();
    
    // 调试相关
    print();                // 输出该aio array 的 信息到某文件
    print_all();            // 输出所有aio array 的信息到某文件
    to_file();
    print_to_file();
}
```
相关调用:

```
// 初始化
srv0start::os_aio_init() {
    AIO::start(limit, n_readers, n_writers, n_slots_sync) {
        s_reads = create(LATCH_ID_OS_AIO_READ_MUTEX, n_readers * n_per_seg, n_readers) {
            AIO *array = UT_NEW_NOKEY(AIO(id, n, n_segments));
            array->init();
            return array;
        }
        s_writes = ...;
        s_ibuf = ...;
        s_log = ...;
        s_sync = ...;
    }
}
```

> TODO：
> 1. handler 相关
>

#### LinuxAIOHandler

---

```
class LinuxAIOHandler {

  dberr_t poll(fil_node_t **m1, void **m2, IORequest *request);
 private:
  dberr_t resubmit(Slot *slot);
  dberr_t check_state(Slot *slot);
  bool is_shutdown() const;
  // n_pending，从该线程负责的第一个 slot 开始查找所有 is_reserved，且 io_already_done 为 false 的slot数，
  // 一旦找到 io_already_done 为 true就停止。
  //  返回第一个 is_reserved，且 io_already_done 都为 true 的 slot
  Slot *find_completed_slot(ulint *n_pending); 
  void collect();

 private:
  AIO *m_array;
  ulint m_n_slots;
  ulint m_segment;
  ulint m_global_segment;
};
```
> TODO：
> 1. resubmit
> 2. check_state
> 3. is_shutdown
> 4. poll
> 5. collect

---

#### AIOHandler 
封装了一些 os_file_ 开头的全局函数，供异步 IO 使用。
```
class AIOHandler {
 public:
  static dberr_t post_io_processing(Slot *slot);
  // 封装了 os_file_io_complete 函数，为 aio 提供的。
  static dberr_t io_complete(const Slot *slot);
 private:
  // 是否被加密
  static bool is_encrypted_page(const Slot *slot);
  // 是否被压缩
  static bool is_compressed_page(const Slot *slot);
  // 返回压缩后的数据大小 + FIL_PAGE_DATA(文件头固定长度38)
  static ulint compressed_page_size(const Slot *slot);
  // 被读上来的长度是否能够达到压缩长度，能的话就能解压
  static bool can_decompress(const Slot *slot);
  static dberr_t check_read(Slot *slot, ulint n_bytes);
};
```


---

#### SyncFileIO
用于同步读，对一些全局函数做了封装。  
被 os_file_io 调用。

---

#### 函数
```
os_is_o_direct_supported        // 是否支持直接读写（绕过内核缓存）
buf_flush_page_cleaner_is_active // buf/buf0flu 下的函数

// os_file_ 开头函数
os_file_punch_hole
os_file_io_complete
os_file_io
os_file_pread
os_file_pwrite
os_file_write_page
os_file_write_func
同步读写大致就是以上几个函数，自下向上调用的过程。

// 加密解密相关
os_file_compress_page
os_file_encrypt_page
os_file_encrypt_log

os_file_lock                    // 对文件上锁，进程锁

// more os_file_ 开头函数...

// Block 分配释放
os_alloc_block
os_free_block

// os_aio_开头
os_aio_linux_handler
os_aio_handler
fil0fil::fil_aio_wait() {
    os_aio_handler() {
        os_aio_linux_handler();
    }
}

os_aio_init
os_aio_free
os_aio_func
os_aio_validate;                // 没啥用
```

---








