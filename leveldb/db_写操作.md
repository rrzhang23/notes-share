## 写操作接口
levedb提供了三种写操作：
* `Put`
* `Delete`
* `Write`

其中 `Put` 和 `Delete` 都是通过调用 `Write` 实现的。
`Put` 和 `Delete` 实现较为简单：
```cpp
    Status DBImpl::Put(const WriteOptions &o, const Slice &key, const Slice &val) {
        return DB::Put(o, key, val);
    }
    Status DB::Put(const WriteOptions &opt, const Slice &key, const Slice &value) {
        WriteBatch batch;
        batch.Put(key, value);   //仅仅放到 WriteBatch::rep_ 中去
        return Write(opt, &batch);
    }

    Status DBImpl::Delete(const WriteOptions &options, const Slice &key) {
        return DB::Delete(options, key);
    }
    Status DB::Delete(const WriteOptions &opt, const Slice &key) {
        WriteBatch batch;
        batch.Delete(key);
        return Write(opt, &batch);
    }

```
具体调用关系如下：
* `leveldb::DBImpl::Put() -> leveldb::DB::Put() -> leveldb::DBImpl::Write()`

* `leveldb::DBImpl::Delete() -> leveldb::DB::Delete() -> leveldb::DBImpl::Write()`

这里每次写入都会定义一个栈空间 `WriteBatch`，后面 `Write` 函数会将这些 `WriteBatch` 合并到一个中去。具体 `WriteBatch` 实现可参考 [WriteBatch](http://note.youdao.com/noteshare?id=914ab2bed478e59d0ba684b9521d5327) 。  
另外和其他操作一样，每次参数都会传入一个 `WriteOptions`。这里不做过多叙述，具体可查看 `include/leveldb/options.h` 中的定义。

## DBImpl::Write()

```cpp
1.
Status DBImpl::Write(const WriteOptions &options, WriteBatch *my_batch) {
        Writer w(&mutex_);
        w.batch = my_batch;
        w.sync = options.sync;
        w.done = false;
```
函数先根据传入的 `WriteBatch` 构造一个`Writer w`，使用 `DB::Impl::mutex` 进行初始化。 结构体定义如下：
```cpp
                    struct DBImpl::Writer {
                        Status status;
                        WriteBatch *batch;
                        bool sync;
                        bool done;
                        port::CondVar cv;
                
                        explicit Writer(port::Mutex *mu) : cv(mu) {}
                    };
```
```cpp
2.
        MutexLock l(&mutex_);
        writers_.push_back(&w);     //std::deque<Writer *> writers_
        while (!w.done && &w != writers_.front()) {
            w.cv.Wait();
        }
        if (w.done) {
            return w.status;
        }
```
所有 `Put()、Delete()` 操作将在此阻塞, 主线程将 `w` `push` 到 `writers_` 队列中。  
此处线程等待条件为 `(!w.done && &w != writers_.front())`，满足 `if条件` 说明该操作已完成，可以 return 。 

执行到下面代码说明当前 `w` 处于队列 `writers_` 头部，被选中作为执行线程。
```cp
3.
    Status status = MakeRoomForWrite(my_batch == nullptr); 
        uint64_t last_sequence = versions_->LastSequence();
        Writer *last_writer = &w;
```
`MakeRoomForWrite()` 确保 `memtable` 有剩余空间，执行完后，`memtable` 一定会有剩余空间。  
`MakeRoomForWrite()` 函数会在后面讲解。  
`last_writer` 指向队首。

```cpp
4.
    if (status.ok() && my_batch != nullptr) {
            WriteBatch *updates = BuildBatchGroup(&last_writer);
            // 同一组 writers_ 中的 operation 有同一个 sequence
            WriteBatchInternal::SetSequence(updates, last_sequence + 1);
            last_sequence += WriteBatchInternal::Count(updates);

            {
                mutex_.Unlock();
                status = log_->AddRecord(WriteBatchInternal::Contents(updates));
                bool sync_error = false;
                if (status.ok() && options.sync) {
                    // 写入日志
                    status = logfile_->Sync();
                    if (!status.ok()) {
                        sync_error = true;
                    }
                }
                if (status.ok()) {
                    // 提交至 mem_
                    status = WriteBatchInternal::InsertInto(updates, mem_);
                }
                mutex_.Lock();
                if (sync_error) {
                    RecordBackgroundError(status);
                }
            }
```
上面这段 `if` 将 `batch` 批量写入 `log` 和 `memtable` 。  
`BuildBatchGroup(&last_writer);` 从队首开始，把后面的 `batch` 合并到一个 `WriteBatch` 中。调用之后，`last_writer` 暂时指向队尾。  
为什么是“暂时”？  
1. 因为在写入 `log` 和 `memtable` 时会暂时释放锁，这时候队列 `writes_` 是可以继续往后追加值的。  
2. 同时`MakeRoomForWrite` 会短暂释放锁。`writes_` 也可以被访问。
```cpp
5.
        while (true) {
            Writer *ready = writers_.front();
            writers_.pop_front();
            if (ready != &w) {
                ready->status = status;
                ready->done = true;
                ready->cv.Signal(); // 通知 w 返回
            }
            if (ready == last_writer) break;
        }

        // Notify new head of write queue
        // 这里有点疑问, 所没有释放, writers_ 应该为空
        if (!writers_.empty()) {
            writers_.front()->cv.Signal();
        }
```
最后这个 `if` 开始读的时候是存在一些疑问的，为什么会存在 `writers_` 非空的情况？上上段其实已经讲了，暂时释放锁的时候，`writers_` 是可以写入的。  
`while` 循环向队首至 `last_writer` 的操作释放信号，表明这些操作已经完成，可以返回。

## DBImpl::MakeRoomForWrite()
