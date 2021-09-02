#### 1. 一些可调的数据库参数 `leveldb::config`
```cpp
namespace config {
        static const int kNumLevels = 7;
        // 触发 Compaction 的第0层 SStable 数量
        static const int kL0_CompactionTrigger = 4;
        static const int kL0_SlowdownWritesTrigger = 8;
        static const int kL0_StopWritesTrigger = 12;
        static const int kMaxMemCompactLevel = 2;
        static const int kReadBytesPeriod = 1048576;
    } 
```

#### 2. key删除或添加标志
```cpp
enum ValueType {
    kTypeDeletion = 0x0,
    kTypeValue = 0x1
};
```
`SequenceNumber` 以及 `kMaxSequenceNumber`：
```cpp
static const ValueType kValueTypeForSeek = kTypeValue;
// 实际只占7字节的最大SequenceNumber, kMaxSequenceNumber二进制如下
// 0000000011111111111111111111111111111111111111111111111111111111
static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);   
```

#### 3. InternalKey、LookupKey及其相关函数。
***InternalKey: 结构***
```
                            7 bytes           1 byte
+-------------------------+------------------+----------------------+
| user_key.data()         | sequence         | log ValueType        |
+-------------------------+------------------+----------------------+
```
前面是 `key` 的数据部分, 接着是一个七字节的 `sequence`，最后一字节的 `ValueType` 表示删除或添加。    

**关于 `InternalKey` 的结构和操作：**  

* struct ParsedInternalKey;  // InternalKey 的结构体  
* class InternalKey;    // InternalKey初始化等操作  
>这里要主要注意 `ParsedInternalKey` 和 `ParseInternalKey`, 一个是结构体，后者是一个函数，用于解析该结构体：  
>`inline bool ParseInternalKey(const Slice &internal_key,                             ParsedInternalKey *result)`
```cpp
    struct ParsedInternalKey {
        Slice user_key;
        SequenceNumber sequence;
        ValueType type;
    };
    
    class InternalKey {
    private:
        std::string rep_;
    public:
        
        InternalKey() {};    // 空 rep_ 表示不合法的
        InternalKey(const Slice &user_key, SequenceNumber s, ValueType t) {
            // 添加一个 InternalKey 到 rep_ 后，这里 rep_应该只是一个 InternalKey, 不会一连串存放好几个 InternalKey
            AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
        }

        // DecodeFrom 和 Encode 仅仅是 string 和 Slice 之间的转换
        void DecodeFrom(const Slice &s) { rep_.assign(s.data(), s.size()); }
        Slice Encode() const { assert(!rep_.empty());  return rep_; }

        // 即返回 InternalKey 中真正的 key(去除最后 8 个字节)
        Slice user_key() const { return ExtractUserKey(rep_); }

        // 用其他 InternalKey 初始化当前 rep_
        void SetFrom(const ParsedInternalKey &p) {
            rep_.clear();
            AppendInternalKey(&rep_, p);
        }
        void Clear() { rep_.clear(); }
    };
```
`class InternalKey` 内部使用一个 `rep_:string` 来记录 InternalKey。  
#### 4. 用于比较 InternalKey 的比较器
```cpp
    class InternalKeyComparator : public Comparator {
    private:
        const Comparator *user_comparator_;    // 应该是 BytewiseComparatorImpl
    public:
        explicit InternalKeyComparator(const Comparator *c) : user_comparator_(c) {}
        // return "leveldb.InternalKeyComparator";
        virtual const char *Name() const;
        // 先用 BytewiseComparatorImpl 比较  InternalKey.user_key，再比较 InternalKey.sequence
        virtual int Compare(const Slice &a, const Slice &b) const;
        // 还是先抽取出 InternalKey.user_key，用字节比较器比较，返回最短公共字符串，由 start 带出，里面还有一个关于 sequence 的细节处理
        virtual void FindShortestSeparator(
                std::string *start,
                const Slice &limit) const;
        // 不大看得懂
        virtual void FindShortSuccessor(std::string *key) const;
        const Comparator *user_comparator() const { return user_comparator_; }
        int Compare(const InternalKey &a, const InternalKey &b) const;
    };
```


#### 5. 用于 InternalKey 的 FilterPolicy
```cpp
    class InternalFilterPolicy : public FilterPolicy {
    private:
        const FilterPolicy *const user_policy_;
    }
```
内部提供两个操作：  

* CreateFilter();
* KeyMayMatch();

都是抽取 `InternalKey.user_key`，然后再通过调用 `user_policy_` 实现先关功能。  

#### 6. LookupKey
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