#### 1. PosixSequentialFile类
先看接口 `SequentialFile`
```cpp
    virtual Status Read(size_t n, Slice *result, char *scratch) = 0;
    virtual Status Skip(uint64_t n) = 0;
```

PosixSequentialFile定义:
```cpp
// 成员变量：
    const int fd_;
    const std::string filename_;
// 成员函数：
    Status Read(size_t n, Slice *result, char *scratch) override;
    Status Skip(uint64_t n) override;
```

`Read()` 提供了POSIX的读功能，调用 POSIX read 函数实现：
```cpp
::ssize_t read_size = ::read(fd_, scratch, n);
```

Skip调用POSIX lseek：
```cpp
::lseek(fd_, n, SEEK_CUR) == static_cast<off_t>(-1)
```


#### 2. PosixRandomAccessFile类
继承自接口：`RandomAccessFile`
```cpp
// 成员变量：
    const bool has_permanent_fd_;  // If false, the file is opened on every read.
    const int fd_;  // -1 if has_permanent_fd_ is false.
    Limiter *const fd_limiter_;  //改参数在 POSIXENV::NewRandomAccessFile()函数传入
    const std::string filename_;
// 成员函数：
    virtual Status Read(uint64_t offset, size_t n, Slice *result,   char *scratch) const = 0;
```
内部实现调用了 POSIX 的 pread 原子读取函数:
```cpp
pread(fd, scratch, n, static_cast<off_t>(offset));
```
这里对 `uint64_t` 类型做了一个 `static_cast<off_t>` 转换，`off_t` 定义在 `<fcntl.h>` 头文件中。 

#### 3. PosixWritableFile类
```cpp
//成员:
    char buf_[kWritableFileBufferSize]; //缓冲区 长度 65536
    size_t pos_;
    int fd_;
    const bool is_manifest_;  // True if the file's name starts with MANIFEST.
    const std::string filename_;
    const std::string dirname_;  // The directory of filename_.
//函数：    
    //无缓冲， 直接调用 ::write(fd_, data, size)，写入文件
    Status WriteUnbuffered(const char *data, size_t size);
    // 先同步DirIfManifest，再刷新缓冲区
    Status Sync() override {
        Status status = SyncDirIfManifest();
        status = FlushBuffer();
    }
    //Append函数尽可能写入 buffer
    Status Append(const Slice &data) override;
```

#### 4. PosixEnv类
继承自ENV，通过实例化上面几个类，实现随机读、序列读、写入等几个函数功能。

#### 5. 其他几个类：
```cpp
* Limiter
* PosixMmapReadableFile   //通过mmap内存映射加速读取
* PosixFileLock
* PosixLockTable
```