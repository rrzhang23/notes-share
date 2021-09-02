### 背景
如果想判断一个元素是不是在一个集合里，一般想到的是将集合中所有元素保存起来，然后通过比较确定。链表、树、散列表（又叫哈希表，Hash table）等等数据结构都是这种思路，存储位置要么是磁盘，要么是内存。很多时候要么是以时间换空间，要么是以空间换时间。  
在响应时间要求比较严格的情况下，如果我们存在内里，那么随着集合中元素的增加，我们需要的存储空间越来越大，以及检索的时间越来越长，导致内存开销太大、时间效率变低。

此时需要考虑解决的问题就是，在数据量比较大的情况下，既满足时间要求，又满足空间的要求。即我们需要一个时间和空间消耗都比较小的数据结构和算法。Bloom Filter就是一种解决方案。

**更多资料可自行搜索。**

[布隆过滤器(Bloom Filter)详解](https://www.cnblogs.com/liyulong1982/p/6013002.html)  

[大数据量下的集合过滤—Bloom Filter](https://www.cnblogs.com/z941030/p/9218356.html)

### leveldb bloomfilter实现：  
`leveldb` 把过滤器信息和 `table` 放在一起，没有采用 `bitmap` 结构实现 `bloomfilter` ，而是将比特位的数据存放在一个 `Slice` 中，这样便于存放于 `SSTable`，而不需要采用其他特定结构存放。  

`bloomfilter` 实现定义在 `bloom.cc` 中，成员变量:
```cpp
        private:
            size_t bits_per_key_;   // 每个键采用多少个比特位记录
            size_t k_;              // 哈希函数的个数
```

函数：
```cpp
1.
    explicit BloomFilterPolicy(int bits_per_key) : bits_per_key_(bits_per_key) {
                k_ = static_cast<size_t>(bits_per_key * 0.69);
            }
```
1. 构造函数，没什么可讲。这里 `0.69` 是一个经验值，具体可参考 `bloomfilter` 博客讲解。

```cpp
2.
    virtual void CreateFilter(const Slice *keys, int n, std::string *dst) const;
```

2. 采用批量的方式建立一个过滤器，在构建 `table` 时（[table相关讲解]()），大约根据 2K 左右的 `entries` 调用该函数构建一次 `filter`。  
传入 `keys[], keys.size()`, 以及一个也许存储了部分过滤信息的字符串 `*dst`，用于存放过滤器的比特位。  
具体实现是先根据 `keys.size()` `resize` dst 容量，然后把该批 `entries` 的哈希信息存储到扩展的容量对应位置上。


```cpp
3.   
            
        virtual bool KeyMayMatch(const Slice &key, const Slice &bloom_filter) const;
            
    
// 这个函数较为简单，仅仅是给定一个 key 和一个bloom_filter，
            // 返回 bloom_filter 是否存放了该 key
```
3. 这个函数较为简单，仅仅是给定一个 `key` 和一个 `bloom_filter`，返回 `bloom_filter` 是否存放了该 `key`。

```cpp
4.
    const FilterPolicy *NewBloomFilterPolicy(int bits_per_key) {
        return new BloomFilterPolicy(bits_per_key);
    }
```
4. 该函数没有继承自 `FilterPolicy`，仅仅是 `FilterPolicy` 中函数声明的实现


