#### dbformat.h
1. ***InternalKey:***
```
                            7 bytes           1 byte
+-------------------------+------------------+----------------------+
| user_key.data()         | sequence         | log ValueType        |
+-------------------------+------------------+----------------------+
```
***更多分析见 dbformat.md***



2. ***LookupKey：***
这个类构造一个“特定”字符串用于 `DBImpl::Get()`  
其包含四个成员：  
```cpp
        const char *start_;
        const char *kstart_;
        const char *end_;
        char space_[200];	// 若空间不够，可动态增长
```
这几个成员和“特定”字符串的关系如下：  
```cpp
start                              kstart             end
+----------------------------------+------------------+
| InternalKey.size():varint32      | InternalKey      |
+----------------------------------+------------------+
```
由函数：
```cpp
Slice memtable_key() const { return Slice(start_, end_ - start_); }
```
可知，`memtable` 内存储的 `InternalKey`格式不只是 `InternalKey` 格式，还在 `InternalKey` 前面加上了一个32位可变长编码的长度值。