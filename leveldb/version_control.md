## 前言
建议这篇文章结合[db_compction & version_control]()一起来看，[db_compction & version_control]()详细描述了 `BD 合并操作` 是怎么和  `版本控制`结合在一起的。  

leveldb 的版本控制在[这篇文章](https://segmentfault.com/a/1190000009707717#articleHeader12)介绍的比较详细：  
https://segmentfault.com/a/1190000009707717#articleHeader12

>实际上上述文章介绍了 leveldb 的整体情况，很详尽。

这篇文章只是作为笔记，根据上述笔记及文章讲的内容，结合代码看看 `version control` 在 leveldb 是怎么实现的。

## 概要
`version` 涉及到四个类以及一个结构体，分别定义在 `version_set`、`verson_edit` 当中：  
`version_set`：  
1. `class Version`
2. `class VersionSet`
3. `class Compaction`  

`verson_edit`：
4. `class VersionEdit`
5. `struct FileMetaData`

### struct FileMetaData
`struct FileMetaData` 作为一个文件的描述结构，定义如下，具体含义看注释：
```cpp
    struct FileMetaData {
        int refs;
        int allowed_seeks;          // 初始化为 2^30, Seeks allowed until compaction
        uint64_t number;            // 文件号，递增，和日志共用一套
        uint64_t file_size;         // 文件字节数
        InternalKey smallest;       // table 中最小的 key
        InternalKey largest;        // table 中最大的 key

        FileMetaData() : refs(0), allowed_seeks(1 << 30), file_size(0) {}
    };
```

### class VersionEdit
**`VersionEdit` 主要用于构造 `Manifest` 文件。** 一个 `Manifest` 结构图如下：  
![][Manifest]

`Manifest` 定义了 8 种 db 元数据。  
数据库每产生一次合并就会有一次状态的变更，`VersionEdit` 描述了一次 db 状态变更，而最为直接的持久化和apply它的办法就是：  
* 构造 VersionEdit 并写入 Manifest 文件
* 合并当前 Version 和 versionEdit 得到新 version 加入 versionSet
* 将当前 version 指向新生成的 version

#### 成员变量
扒开 `VersionEdit` 代码看看：
对应上述 8 种状态，定义了如下 8 个变量： 
```cpp
        std::string comparator_;
        uint64_t log_number_;
        uint64_t prev_log_number_;
        uint64_t next_file_number_;
        SequenceNumber last_sequence_;

        std::vector<std::pair<int, InternalKey> > compact_pointers_;
        typedef std::set<std::pair<int, uint64_t> > DeletedFileSet;
        DeletedFileSet deleted_files_;
        std::vector<std::pair<int, FileMetaData> > new_files_;
```
从定义中可以看出，后面三种状态分别以 `pair` 二元组记录一个 `versoin` 到另一个 `version` 的变化 1. 合并指针、2. 添加的文件、3. 删除的文件。

>根据参考文章中描述，这里 version 版本的变化实际上是不同版本下 level 中文件的变更，所以二元组的第一个 int 类型存储的是对应 level。

#### 成员函数
另外，针对以上变量，`VersionEdit` 定义了系列 `Setter()`函数，这里不再赘述。  
值得一提的是，`Manifest` 存储的是字符类型，不能只是简单使用 `toString` 将变量简单转换成 `string` 类型。`VersionEdit` 提供了一下两个函数用于序列和反序列化：
```cpp
        void EncodeTo(std::string *dst) const;
        Status DecodeFrom(const Slice &src);
```
#### more
`VersionEdit` 还提供了针对调试使用的 `DebugString()` 函数。  
和一个枚举变量 `leveldb::tag`：
```cpp
    enum Tag {
        kComparator = 1,
        kLogNumber = 2,
        kNextFileNumber = 3,
        kLastSequence = 4,
        kCompactPointer = 5,
        kDeletedFile = 6,
        kNewFile = 7,
        // 8 was used for large value refs
                kPrevLogNumber = 9
    };
```

### class Compaction
根据代码中注释：  
`A Compaction encapsulates information about a compaction.`  
即 **`Compaction` 记录了某个 `version` 某次合并的信息。**  

在 `class DBImpl` 中使用时，进一步被封装到 `struct DBImpl::CompactionState` 中去：  
```cpp
    struct DBImpl::CompactionState {
        Compaction *const compaction;
        ...
    }
```

#### 成员变量
```cpp
        int level_;                     // 表示当前层 level[i], 将和 level[i+1] 层合并
        uint64_t max_output_file_size_; // 文件最大大小，2MB (options->max_file_size;)
        Version *input_version_;        // 表示该次合并实在某个版本下产生的
        VersionEdit edit_;              // 只在 VersionSet::SetupOtherInputs(Compaction *c) 中使用到

        std::vector<FileMetaData *> inputs_[2];      // 记录 level[i]、level[i+1] 层需要合并的文件

        // 用于存放合并之后，level[i+2]层 和 outputfile 有数据重叠的那些文件
        // (parent == level_ + 1, grandparent == level_ + 2)
        std::vector<FileMetaData *> grandparents_;  // grandparent == level_+2
        size_t grandparent_index_;  // Index in grandparent_starts_
        bool seen_key_;             // Some output key has been seen
        int64_t overlapped_bytes_;  // outputfile 和 grandparents_ 重叠的字节数

        size_t level_ptrs_[config::kNumLevels]; // kNumLevels = 7
```
#### 成员函数
```cpp
Getter():
        int level() const { return level_; }
        VersionEdit *edit() { return &edit_; }
        int num_input_files(int which) const { return inputs_[which].size(); }
        FileMetaData *input(int which, int i) const { return inputs_[which][i]; }
        uint64_t MaxOutputFileSize() const { return max_output_file_size_; }

        // 判断当前层是否能直接放到下一层, 没有合并操作和键值范围重合
        // 判断条件为:
        //      当前层数 level 文件数量为 1
        // &&   下层 level+1 文件数数量为 0
        // &&   level + 2 层所有文件站磁盘总字节数 小于 20MB(10*options->max_file_size)
        bool IsTrivialMove() const;

        // 把 level、level+1 层所有的文件标记为删除,
        // 函数的作用就是把 level、level+1 层所有的文件
        // 以 <level, FileMetaData.number> 格式添加到 VersionEdit.deleted_files_ 中
        void AddInputDeletions(VersionEdit *edit);

        // 对于给定的 internal_key, 该函数确保该 internal_key 不会出现在 level[i] 层(i>=level+2) 当中。
        bool IsBaseLevelForKey(const Slice &user_key);

        // 判断 output 是否过大(大于20MB)
        bool ShouldStopBefore(const Slice &internal_key);

        // 释放 input_version_
        void ReleaseInputs();

```
可以看出，`Compaction` 函数大部分是 Getter()，和一些“level"级的优化函数（`class Version` 里面则更多是针对 `level[i]` 中文件和键值对范围的优化函数），然后还有一个 ReleaseInputs()。

### class Version
```cpp
        VersionSet *vset_;            // 指明该 Version 属于哪个 VersionSet
        Version *next_;               // 后项指针
        Version *prev_;               // 前向指针
        int refs_;                    // 引用计数

        //每层 level 的文件集合
        std::vector<FileMetaData *> files_[config::kNumLevels];

        // 用于判断是否执行 DB::Impl::MaybeScheduleCompaction()
        // TODO: 但还是不太明白
        FileMetaData *file_to_compact_;
        int file_to_compact_level_;

        // Level that should be compacted next and its compaction score.
        // Score < 1 means compaction is not strictly needed.  These fields
        // are initialized by Finalize().
        double compaction_score_;
        int compaction_level_;
```

```cpp

// for lookup:
        // *iters 指向一组迭代器，迭代各层的文件
        // 具体指向：
        //      iter[0] -> files_[0][0]
        //      iter[i] -> files_[0][i]
        //      iter[n] -> files_[0][n]
        //      iter[n+1] -> files_[1]
        //      iter[n+i] -> files_[i] (i>1)
        //      iter[n+3] -> files_[n]
        // 这里 files_[0][i]、files_[i] 并不是文件，而是这个迭代器指向的二级迭代器
        // 二级迭代器的概念见 table 中的定义
        void AddIterators(const ReadOptions &, std::vector<Iterator *> *iters);

        // 给定 key,查找 value
        // 先查找 level[0],然后 level[i] (i>0)
        // TODO: stats 不清楚干嘛用
        Status Get(const ReadOptions &, const LookupKey &key, std::string *val, GetStats *stats);
                   
// for DB::Impl::MaybeScheduleCompaction():                   
        struct GetStats {
            FileMetaData *seek_file;
            int seek_file_level;
        };

        // TODO:
        // 用于判断是否执行 DB::Impl::MaybeScheduleCompaction()
        bool UpdateStats(const GetStats &stats);
        // TODO:
        // 用于判断是否执行 DB::Impl::MaybeScheduleCompaction()
        bool RecordReadSample(Slice key);
        
        // Version::RecordReadSample() 会用到
        void ForEachOverlapping(Slice user_key, Slice internal_key,
                                void *arg,
                                bool (*func)(void *, int, FileMetaData *));

// 引用计数
        void Ref();
        void Unref();
        
// 优化函数：
        // 给定 begin、end, 返回该层中，那些文件和这个范围内的数据项有重合
        // 返回的是 inputs
        void GetOverlappingInputs(
                int level,
                const InternalKey *begin,         // nullptr means before all keys
                const InternalKey *end,           // nullptr means after all keys
                std::vector<FileMetaData *> *inputs);

        // Returns true iff some file in the specified level overlaps
        // some part of [*smallest_user_key,*largest_user_key].
        // smallest_user_key==nullptr represents a key smaller than all the DB's keys.
        // largest_user_key==nullptr represents a key largest than all the DB's keys.

        // 返回 [*smallest_user_key, *largest_user_key] 是否存在于 level 中
        bool OverlapInLevel(int level,
                            const Slice *smallest_user_key,
                            const Slice *largest_user_key);

        // Return the level at which we should place a new memtable compaction
        // result that covers the range [smallest_user_key,largest_user_key].

        // DB::Impl::WriteLevel0Table() 在讲 memtable 刷盘时，先构建 FileMetaData,写入 memtable 数据
        // 然后这个 FileMetaData 必然要添加到某个 version 的 level 和 files_ 中去
        // 这里并没有直接写入 level[0]
        //      而是先判断 memtable 中的数据和 0 层有没有重合，
        //          若有，则添加到 0 层，后面要compation
        //          若没有，找到合适的 i 层(i > 0),这里合适指 memtable 和 i 层 也没有重合，则直接放到 i 层
        int PickLevelForMemTableOutput(const Slice &smallest_user_key,
                                       const Slice &largest_user_key);
// Getter():
        int NumFiles(int level) const { return files_[level].size(); }

        // 调试输出用
        std::string DebugString() const;


// for Version::AddIterators():
        class LevelFileNumIterator;
        // 返回 level[i] (i>0) 二级迭代器
        Iterator *NewConcatenatingIterator(const ReadOptions &, int level) const;


```

这里每个函数的功能都加了注释，大概分为几类：  
* 数据读取相关
* 给 DB::Impl::MaybeScheduleCompaction() 提供判断, 这里state部分还是有点疑问
* compaction 优化相关，尽管 `Compaction` 里面已经有了一些优化函数。
* ref:引用计数
* Getter()
* 调试

值得关注的是 Version::state及其相关操作。  

另外之前固有的印象就是，`memtable` 都是直接放到 0 层，但是这里提供了函数 `PickLevelForMemTableOutput()`，选择合适的层级，然后直接放到该层中。

对于数据库的读取和scan，这里都提供了相关的函数操作。尤其是 迭代器的实现，和 table 中二级迭代器呼应上了。光看 table 中二级迭代器是不能详细理解它的用法的。

### class VersionSet
顾名思义，上面 `Version` 类拥有一个成员变量指明该 `Version` 属于哪个 `VersionSet`，说明 `VersionSet` 是一组 `Version` 的集合。  
参考文章中提到，这些 Versions 是通过双向链表连接起来的。

#### 成员变量
```cpp
        Env *const env_;
        const std::string dbname_;
        const Options *const options_;
        TableCache *const table_cache_;
        const InternalKeyComparator icmp_;
        
        uint64_t next_file_number_;
        uint64_t manifest_file_number_;     // manifest 文件号
        
        uint64_t last_sequence_;            // 最后时间戳
        
        uint64_t log_number_;               // 日志文件号
        uint64_t prev_log_number_;          // 0 or backing store for memtable being compacted

        // 写入 MANIFEST 用到的参数
        WritableFile *descriptor_file_;
        log::Writer *descriptor_log_;
        Version dummy_versions_;  // 仅用作链表头部，不存储实际 files
        Version *current_;        // 当前使用的 version，== dummy_versions_.prev_

        // 每一层的 compaction 应当从这个 InternalKey 开始
        // 要么为空 string，要么是合法的 InternalKey
        std::string compact_pointer_[config::kNumLevels];  // kNumLevels = 7
```



#### 成员函数
```cpp
// MANIFEST 相关函数
        Status LogAndApply(VersionEdit *edit, port::Mutex *mu) EXCLUSIVE_LOCKS_REQUIRED(mu);
        Status Recover(bool *save_manifest);
        // TODO:
        bool ReuseManifest(const std::string &dscname, const std::string &dscbase);
        // Save current contents to *log
        Status WriteSnapshot(log::Writer *log);
        
        
// 通过调用 current version 实现的函数：         
        int NumLevelFiles(int level) const;
        int64_t NumLevelBytes(int level) const;
// about FileNumber
        void ReuseFileNumber(uint64_t file_number);
        void MarkFileNumberUsed(uint64_t number);
// about Sequence
        void SetLastSequence(uint64_t s);
        
        
// 和 compaction 对象相关，挑选合并文件及合并范围        
        // TODO:
        Compaction *PickCompaction();
        Compaction *CompactRange(
                int level,
                const InternalKey *begin,
                const InternalKey *end);
        bool NeedsCompaction() const {
            Version *v = current_;
            return (v->compaction_score_ >= 1) || (v->file_to_compact_ != nullptr);
        }
        // 从某个 version 中选出最需要合并的 level
        void Finalize(Version *v);
        // 给定某层 level 的文件 inputs,
        // 返回最小的 smallest 和最大的 largest InternalKey
        void GetRange(const std::vector<FileMetaData *> &inputs, InternalKey *smallest, InternalKey *largest);
        // 同样给定 2 个inputs,
        // 返回最小的 smallest 和最大的 largest InternalKey
        // 内部调用 GetRange 实现
        void GetRange2(const std::vector<FileMetaData *> &inputs1, const std::vector<FileMetaData *> &inputs2, InternalKey *smallest, InternalKey *largest);
        // TODO:
        void SetupOtherInputs(Compaction *c);
        
// 创建当前 inputfiles 文件中 entries 的迭代器
// 和 merger.h 中 NewMergingIterator() 呼应上
        Iterator *MakeInputIterator(Compaction *c);        
        
        
        // TODO:
        int64_t MaxNextLevelOverlappingBytes();

        // 这里 live 指系统中现有所有文件
        // 迭代每个 version, 然后把 version.files_ 中的所有文件号添加到 lives 中
        void AddLiveFiles(std::set<uint64_t> *live);

        // 返回某个 Version 版本中 key 对应的偏移量
        // 这个偏移量为之前所有小于 key 的文件大小 加上 该 key 所在 SSTable 内的偏移量
        uint64_t ApproximateOffsetOf(Version *v, const InternalKey &key);

        struct LevelSummaryStorage {
            char buffer[100];
        };
        // 返回各层 level 包含的 SSTable 文件数量
        // 示例:
        //      level[ 1 2 3 4 5 6 7 ]
        const char *LevelSummary(LevelSummaryStorage *scratch) const;

        // 在双向链表最后添加 Version 节点。current_ 指向新添加的节点。
        void AppendVersion(Version *v);
        
// Getter():
        ...
```
一个 `class VersioSet` 大致如上，成员变量和函数用法都有注释。

## 写在最后
[db_compction & version_control]() 这篇文章详细讲述了二者之间的联系，和 `版本控制` 在数据库中的用法。我们把文章总结搬过来看一下，然后再补充点 `版本控制` 里面几个组件的联系和区别。  
>总结  
>version control 和 DB Compaction 合并操作是两个紧密相连的部件，一个 DB 实例拥有一个 VersionSet 对象（双向链表结构） versions_ 用于记录每次合并的更改记录。  
>而每次合并创建的 class Compaction 对象 compaction 记录了本次合并的一切文件改动，然后 version_ 通过调用 VersionSet::LogAndApply() 将改动以一个 version 对象记录下来。  
>LogAndApply() 具体做法是先补全 compaction->edit 的成员变量，然后创建新的 version 节点 v，用 VersionSet::Builder 来将 compaction->edit 的改动记录应用到 v 中；接着写入 MANIFEST 文件；最后添加 v 到 链表 versions_ 尾部，versions_->current = v。

其实看完整个介绍，对于 `VersionEdit`、`Version`、`VersionSet`、`Compaction`还是有些混淆。  

我们从 `class Compaction` 入手，它是每次 DB 合并操作都会创建的对象，记录了 level、level+1 层的输入文件，以及合并之后的输出文件。  
OK，here comes the problem!  
1. 这些输入文件（需要合并的 SSTable 文件），怎么记录？
2. 这些输入文件以及这个 level 从何而来？

针对问题 1. leveldb 定义了一个 `class VersionEdit` 用于存放这样的输入输出。它定义了几个 vector<pair> 成员变量用于存放输入文件、输出文件、compact_pointers_。  

针对问题 2. leveldb 定义了 `class Version` 和 `class VersionSet` 两个类。  
`class Version` 是 `class VersionSet` 链表的一个节点，拥有前向指针和后向指针。同时还拥有以下几个变量:  
* FileMetaData *file_to_compact_
* int file_to_compact_level_
* double compaction_score_
* compaction_level_

有何用处？  
笔记 [db_compction & version_control]() 里提到三种触发合并的时机，这里前两个便是用于 **`Seek触发`**，后两个用于 **`容量触发`**。  
那么，针对问题 2. 是否只要 `class Version` 就够了呢？为什么还需要 `class VersionSet`？  
理论上是，但是为了实现多版本及宕机和恢复，我们一个 BD 实例只拥有一个 `versions_:VersinoSet` 对象，所有对于合并需要的信息只能通过 `versions_` 调用。所以问题 2. ，VersionSet 封装了一些操作。事实上，很多函数还是通过 `versions_->current_version` 来实现的。这里 BD::Impl 只需要关注 versions_ 就行。

剩下的流程我想上面的总结写的够清楚错了。

还需要交代的就是：
1. `Compaction` 对于 level 层面做了一些优化。
2. `Version` 对于文件层面做了一些优化。
3. `Version` 和 `VerionSet` 有很多相似之处，这也很好理解，`VersinoSet` 很多时候还是调用 `versions_->current_version` 实现部分功能的。
4. `VersionSet` 和 `VersionEdit` 也有很多相似之处，体现在二者同时拥有很多一样的成员变量上。原因是 DB::Impl 里构建的 `Compaction->edit` 只初始化了部分参数，剩下的很多参数需要全局唯一的 versions_ 参数来初始化。
5. `class VersionSet` 还有一些关于 `MANIFEST` 文件的读取和写入操作，用于数据库的宕机和恢复。
6. `class VersionSet` 不只是通过调用 current_version 来实现 level 的挑选。还提供了系列函数，这些函数针对给定的键值对范围 [start_key, limit_key], 找出对应的 level 及对应的文件。

### 遗留的问题

关于合并时，输入文件的 entries 键值对的迭代器是怎么实现的。  
可以参考:  

[table_merger 用法]()  
[table_二级迭代器用法]()

[Manifest]:data:image/jpeg;base64,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