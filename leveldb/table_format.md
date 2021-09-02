## 一个 table 文件组织结构图简单翻译：
```cpp
data blocks:
+----------------------------------+----------------+---------------+
| BlockContents[1].data : string   | type : uint8   | crc :uint32   |
+----------------------------------+----------------+---------------+
| BlockContents[2].data : string   | type : uint8   | crc :uint32   |
+----------------------------------+----------------+---------------+
...
+----------------------------------+----------------+---------------+
| BlockContents[n].data : string   | type : uint8   | crc :uint32   |
+----------------------------------+----------------+---------------+
filter block:
+---------------------+
| filter[1] : string  |
+---------------------+
| filter[2] : string  |
+---------------------+
...
+---------------------+
| filter[n] : string  |
+---------------------+
+----------------------+----------------------+-------+----------------------+-----------------------------------+---------------+
| offset[1] : uint32   | offset[2] : uint32   | ...   | offset[n] : uint32   | sum(sizeof(filter[i])) : uint32   | baselg : uint8|
+----------------------+----------------------+-------+----------------------+-----------------------------------+---------------+
metaindex block:
+---------------+-------------------------------+----------------+---------------+
| filte_name    | Encoded filter_block_handle   | type : uint8   | crc :uint32   |
+---------------+-------------------------------+----------------+---------------+
index blocks:
+--------------------------------------+--------------------------------+----------------+---------------+
| BlockContents[1].last_key : string   | Encoded data_block[1]_handle   | type : uint8   | crc :uint32   |
+--------------------------------------+--------------------------------+----------------+---------------+
| BlockContents[2].last_key : string   | Encoded data_block[2]_handle   | type : uint8   | crc :uint32   |
+--------------------------------------+--------------------------------+----------------+---------------+
...
+--------------------------------------+--------------------------------+----------------+---------------+
| BlockContents[n].last_key : string   | Encoded data_block[n]_handle   | type : uint8   | crc :uint32   |
+--------------------------------------+--------------------------------+----------------+---------------+
footer:
+--------------------------------------------+-----------------------------------------------+-------------------+
| Encoded metaindex_block_handle : 10 bytes  | Encoded index_block_handle : 10 bytes   | MagicNumber : 8 bytes   |
+--------------------------------------------+-----------------------------------------------+-------------------+
```

### BlockHandle
结合上图和笔记 [table]() **构建**一小节，我们了解到一个 table 文件在构建过程中有一共写入了四个 block 和一个 footer。这些 block 在文件中的偏移量和长度怎么描述？  

不难发现，上面的图中多次出现了 `block_handle` 这样的词汇，那么 table/format.h 中就定义了这些handle的结构体，由于几种 block 需要的 handle 结构都相同，所以只有一个以下的类：
```cpp
    class BlockHandle {
    public:
    ...
    private:
        uint64_t offset_;
        uint64_t size_;
    };
```
这个 `BlockHandle` 类有什么用呢？对于 table 层面来说我们知道这些 block 都是不定长的，就是一个简单的字符串（当然会有特定的数据结构描述几种不同的 block），所以用 `BlockHandle` 来描述解析 table 中的 block 很有必要。


### Footer
上图中的footer又是什么呢？  

**一个 SSTable 文件解析的开始。**

如下代码所示：  
```cpp
    class Footer {
    public:
    ...
    private:
        BlockHandle metaindex_handle_;
        BlockHandle index_handle_;
    };
```

它记录了 `metaindex block` 和 `index block` 在文件中的偏移量，末尾还有个用于校验的 `magicnumber`。  

`metaindex block` 和 `index block` 是我们解析 `filter block` 和 `data block` 的入口，具体解析和读取会在[table_filter]()、[table_block]() 中讲。

### BlockContent
另外还有一个 `BlockContents` 来存放解析出来的文件块内容，根据代码不难理解：  
```cpp
    struct BlockContents {
        Slice data;           // Actual contents of data
        bool cachable;        // True iff data can be cached
        bool heap_allocated;  // True iff caller should delete[] data.data()
    };
```
