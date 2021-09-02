之前看 `table` 部分的代码，一直在纠结于 `table` 物理文件的构建，即文件中的 `SSTable` 是怎么写入和读取的，忽略了 `table` 类本身的其他用法。

今天在看DB操作时，只记得 `SSTable` 读取和构建是分开实现的，忽然忘记了内存中的 `table` 是怎样的一个容貌。

看来有时候深入代码也不是个很好的事情，回过头再来看db操作流程时时一直不得要领。大家一直在拿 `“深入浅出” `这个词打趣，之前不以为然，现在想想，想要做到难上加难，当初还是自己太年轻。

废话少说，我们来从另一个角度分析一下 `table` 。

## 起因
今天看 db读取时，除了从 `memtable` 和 `immemtable` 中读取外，最多的应该就是从文件中读取了。  
`leveldb` 实现了一个叫做 `VersionSet` 的版本管理工具，它是一个由多个 `Version` 链接起来的双向链表，每触发一次合并操作，就会往后追加一个 `Version` 节点，不同版本之间的 `Version` 记录了文件之间的变化。  
用的最多的就是 `versions_:VersionSet->current` (当前最新的节点)。那么每个 `Version` 保存了些啥内容呢？   
`Version` 类定义里有这么一行：  
```cpp
        // List of files per level
        std::vector<FileMetaData *> files_[config::kNumLevels];
```
这就是所有层、每层所有文件的元信息向量。  
这些信息是和db实例绑定的，也就是说数据库运行周期中，始终有这么个 `versions_->current->files_` 存在，不然数据库怎么知道哪一层有什么文件有什么数据，没有这个 `files_`, 读取对应的 entry 也就无从说起。

`FileMetaData` 定义如下:
```cpp
    struct FileMetaData {
        int refs;
        int allowed_seeks;          // Seeks allowed until compaction
        uint64_t number;
        uint64_t file_size;         // File size in bytes
        InternalKey smallest;       // Smallest internal key served by table
        InternalKey largest;        // Largest internal key served by table

        FileMetaData() : refs(0), allowed_seeks(1 << 30), file_size(0) {}
    };
```

## DB 数据读取流程  
简单梳理一下从文件读取数据的流程：
```cpp
    DBImpl::Get():
        Version::Get():
            TableCache::Get(): {
                FindTable(file_number, file_size, &handle);
                Table *t = reinterpret_cast<TableAndFile *>(cache_->Value(handle))->table;
                s = t->InternalGet(options, k, arg, saver);
            }
        
```

可以清晰的看到，最终调用的时 `table` 缓存结构，`FindTable()` 这个函数会根据传入的 `file_number、file_size` 去缓存中查找有没有缓存项，若有，直接返回，若没有，则从文件中读取，放入缓存结构，然后再返回。  
这里有两个问题：  
1. table_cache 缓存的是什么玩意儿？整个table数据？
2. 若是缓存整个 table 数据，每个 table 文件 2MB，默认最大缓存1000个，算起来就是2GB左右，会不会很大？

#### 有问题就解决问题
table_cache中定义了这么个玩意儿：  
```cpp    
    struct TableAndFile {
        RandomAccessFile *file;
        Table *table;
    };
```
乍一看，貌似和上面的 FileMetaData 有点像。但还是有区别的。  
这个结构体就是table_cache里缓存的内容，一个table对象，一个文件指针。  
所以并不是我们上面想象的缓存一个表所有内容那样。  
那么...和不缓存有啥区别？需要具体的 entry 不还是要进行 I/O 读取。

但是，这里 RandomAccessFile 在 env_posix.h 文件中是调用 “内存文件映射” 实现的，具体方法还是去查资料吧，这里不赘述了，可以简单理解为这个接口提供了内存级的访问速度。

## 
反思 table 及 block

一个 table 文件的物理组织形式见 [table]() 

