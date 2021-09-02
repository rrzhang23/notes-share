这篇文章主要把 `DB::Impl` 中的 `DB Compaction` 操作和 [`version_control`]() 结合起来加深 `leveldb` 对于合并操作在代码中实现的理解。其他方面的细节不做过多阐述。  

>`version_control` 涉及到 `version_set.h` 和 `version_edit.h` 等文件。  
>另外下文中提到的 VersionSet.h 里的 `class Compaction` 和这里的 `DB Compaction` 操作重名了，阅读时注意区分。

---

一个写入大致流程如下所示:  
`DBImpl::Write` -> `DBImpl::MakeRoomForWrite()` -> `DBImpl::MaybeScheduleCompaction()`。  
也就是说，写入流程到一定时间，会触发合并操作。  

### MaybeScheduleCompaction()
```cpp
    void DBImpl::MaybeScheduleCompaction() {
        ...
        else if (imm_ == nullptr &&
                   manual_compaction_ == nullptr &&
                   !versions_->NeedsCompaction()) { // No work to be done
        }
        else {
            background_compaction_scheduled_ = true;
            BackgroundCall() {
                ...
                else {
                    BackgroundCompaction();
                }
                background_compaction_scheduled_ = false;
        
                // BackgroundCompaction() 只是选出一层，然后合并一层到下一层
                // 可能这一层只是 imm_, 只是调用了 CompactMemTable()(将 imm_ 写入后）就返回
                // 也可能合并 第 i 层后，第 j 层扔然需要合并
                MaybeScheduleCompaction();
                background_work_finished_signal_.SignalAll();
            }
        }
    }
```

合并流程如上代码所示，`MakeRoomForWrite()` 函数调用 `MaybeScheduleCompaction()` 。顾名思义，这里也许会调用最终的 `BackgroundCompaction()` 函数，也许不会，这取决于判断条件是否成立。

从其他文章中（[庖丁解LevelDB之版本控制](https://www.jianshu.com/p/9bd10f32e38c)、[Leveldb二三事](https://segmentfault.com/a/1190000009707717#articleHeader12)）也许会了解到 Compaction **触发时机**一共有三个：  
* **容量触发**DB Compaction，即某层容量大于设定的阈值。
* **Seek触发**DB Compaction，每次读取某个文件中的 entry，会将该文件对应的 allowed_seeks--，当这个值小于 0 ，就会触发合并操作。简单来说，某个文件被反复读取，就会触发合并。
* 手动 DB Compaction  

前两种触发在 `version_control` 会有专门的讲解，但是最后一个人为触发的合并操作，
`MaybeScheduleCompaction()`中倒数第二个 `if` 分支体现了这个人为操作是如何触发的。

在判处其他条件之后，到达合并条件，首先将 `background_compaction_scheduled_` 置为 `true`，然后调用 `BackgroundCall() -> BackgroundCompaction()`，在一次合并操作完成之后，置 `background_compaction_scheduled_` 为 `false` ，再次判断是否还需要合并，继而递归调用 `MaybeScheduleCompaction()`，直到没有任何合并需要，最后释放后台合并完成的信号。

**这里只是合并操作的一角，真正的 `Compaction` 操作在 `BackgroundCompaction()` 函数中才刚刚开始。**

---

### BackgroundCompaction()
```cpp
    void DBImpl::BackgroundCompaction() {
1.
        if (imm_ != nullptr) {
            CompactMemTable();
            return;
        }
2.
        Compaction *c;
        bool is_manual = (manual_compaction_ != nullptr);
        if (is_manual) {
            ManualCompaction *m = manual_compaction_;
            c = versions_->CompactRange(m->level, m->begin, m->end);
            ...
            // 写入状态日志
        } else {
            c = versions_->PickCompaction();
        }
3.
        Status status;
        if (c == nullptr) {
            // Nothing to do
        } else if (!is_manual && c->IsTrivialMove()) {
            // 一点小优化, 可以直接把当前层 level 文件放到下一层
            ...
            c->edit()->DeleteFile(c->level(), f->number);
            c->edit()->AddFile(c->level() + 1, f->number, f->file_size,
                               f->smallest, f->largest);
            status = versions_->LogAndApply(c->edit(), &mutex_);
            if (!status.ok()) {
                RecordBackgroundError(status);
            }
            VersionSet::LevelSummaryStorage tmp;
            Log(); // 写入状态日志
        } else {
            CompactionState *compact = new CompactionState(c);
            status = DoCompactionWork(compact);
        }
        delete c;
4.
        // 人为合并操作的一些扫尾工作
    }
```
#### 代码段 1. 判断是否要合并 `immemtable`。 

#### 代码段 2. 构造了一个 `class Compaction` 对象 `compaction`：  
`class Compaction`，即合并操作需要的信息。若该操作由人为发起，则调用  `CompactRange()`，否则调用 `PickCompaction()`。不管怎样，这两个函数都是由  `DB::Impl::versions_ : VersionSet`调用，然后函数内部实现都是使用 `versions_->current_` 来构建 `class Compaction` 对象的（实际上上述两个函数只初始化了 inputs_[0]， 而 inputs_[1] 和部分优化是通过调用 `VersionSet::SetupOtherInputs()` 实现的）。  
有哪些信息呢？  
* compaction->input_version_ = current_
* compaction->input_version_->Ref();
* compaction->inputs_[0]、inputs_[1]，合并涉及到的文件  

若返回空，即 `compaction == nullptr`，那么代表没有输入文件，或者达不到 `versions_->current_` 的合并条件(见前一节的**触发时机**前两种)。

#### 代码段 3. 部分优化 及 调用 `DoCompactionWork(compact)`  
这里会判断合并操作是否可以优化（是否当前层文件能直接放到下一层）。不可以优化的合并多调用了两层函数：`DoCompactionWork()->InstallCompactionResults()`。但是下面的操作是二者都会执行的。

**重点关注 `version control` 相关操作。**  
阅读 `VersionEdit.h` 及 `VersionSet::class Compaction` 我们知道每个 `Compaction` 对象拥有一个 `VersionEdit` 对象，`VersionEdit` 描述了一次 compaction（数据库的某次合并）涉及到的文件修改（增、删、compact_pointer_修改，当然还有一些其他参数）。  
代码段 3. 中 `compaction` 对象只初始化了 `compact_pointers_`、`DeletedFileSet`、`new_files_` 三个参数，然后就通过：  
`versions_->LogAndApply(c->edit(), &mutex_);`  
将本次合并记录写入到 `Manifest` 文件中。


>`compact_pointers_` 在代码段 2. 中 `CompactRange()->SetupOtherInputs()` or `PickCompaction()->SetupOtherInputs()`就已经初始化。

既然提到了 `VersionSet::LogAndApply()` 那就再扩展一下该函数的作用。  
它接收一个 **VersionEdit 对象**和一个锁 **mutex**。  
前面讲到这个函数把 VersionEdit 的合并记录写入到 `Manifest` 文件中，是怎么做的呢？  
由于这里 DB::Impl 中合并操作构建的 compaction 并非所有参数都初始化，只是描述了部分文件的改动，其他很多参数还是根据全局 VersionSet 对象 Versions_ 中的成员初始化的。什么意思呢？简单理解即 compaciton 和 versions_ 共用了部分对象。 
构建一个完整的 compaction 后，利用 VersionSet::Builder 应用此次合并记录到新的版本 version，写入 `Manifest`, 然后调用 `AppendVersion(v)` 加入 versions_ 链表尾部。
>这里 AppendVersion(v) 函数中会把 versions_->current 指针后移，指向链表尾部。

**mutex** 锁干嘛用的呢？  
我们看看谁调用了 `LogAndApply()`，是  `DB::Impl::versions_`。想要修改这个变量的值，当然要加锁啦。

#### 代码段 4. 人为合并操作的一些扫尾工作  

##### 疑问
相信读到这里仍然有点疑问。`BackgroundCompaction()` 函数好像只是构建了一个  `compaction` （代码段2），记录了一些合并涉及到的文件。然后代码段 3 中调用 `DoCompactionWork(compact)` 就完事了，前面大篇幅讲到的 `LogAndApply()` 也主要是在这个 `DoCompactionWork()->InstallCompactionResults()` 中调用的。  
**OK！毫无疑问，涉及到文件的具体合并、删除、键值对的合并等肯定在 `DoCompactionWork` 这个函数中了，下面进一步解析这个套娃函数**。

---

### DoCompactionWork()
```cpp
    Status DBImpl::DoCompactionWork(CompactionState *compact) {
        
        ...
        
        // 创建指向 compaction 中 inputfiles 文件里的 entries 的迭代器
        Iterator *input = versions_->MakeInputIterator(compact->compaction);
        input->SeekToFirst();
        
        ... // 定义一些下文用到的变量
        
        // 很明显，循环迭代到最后一个 entry
        for (; input->Valid() && !shutting_down_.load(std::memory_order_acquire);) {
        
            ...

            Slice key = input->key();
            if (compact->compaction->ShouldStopBefore(key) &&
                compact->builder != nullptr) {
                // 这个判断条件判断是否要停止合并操作，具体看 VersionSet 文件里的函数定义 
            }

            // 这里判断是否该 input->key() 要输出到合并文件 outputfile 中
            // 比如 type == kTypeDeletion，或者 key.sequence <= compact->smallest_snapshot，都表明不应该输出到 outputfile 里
            bool drop = false;
            if (!ParseInternalKey(key, &ikey)) { // 解析 input->key() 发生错误
            } else {
                if (!has_current_user_key ||
                    user_comparator()->Compare(ikey.user_key,
                                               Slice(current_user_key)) != 0) {
                    // 成功解析 input->key() 到 current_user_key，然后设置部分变量
                }

                // drop = true 的条件
                if (last_sequence_for_key <= compact->smallest_snapshot) {
                    // 解析出来的 sequence 有问题，居然大于 kMaxSequenceNumber，立即放弃（drop = true）
                    drop = true;    // (A)
                } else if (ikey.type == kTypeDeletion &&
                           ikey.sequence <= compact->smallest_snapshot &&
                           compact->compaction->IsBaseLevelForKey(ikey.user_key)) {
                    drop = true;
                }
            }
            ...
            // 将单个 input->key() 输出到文件 outputfile 中
            // 先判断 compact->builder 是否为空，为空代表还没有输出文件，OpenCompactionOutputFile() 需要新建一个文件作为输出
            // 然后 compact->builder 向文件中添加 key/value 对，
            // 要是文件大小到一定程度，调用 FinishCompactionOutputFile() 关闭这个文件
            if (!drop) {
                // Open output file if necessary
                if (compact->builder == nullptr) {
                    status = OpenCompactionOutputFile(compact);
                    if (!status.ok()) {
                        break;
                    }
                }
                if (compact->builder->NumEntries() == 0) {
                    compact->current_output()->smallest.DecodeFrom(key);
                }
                compact->current_output()->largest.DecodeFrom(key);
                compact->builder->Add(key, input->value());

                // 文件达到 2MB
                if (compact->builder->FileSize() >=
                    compact->compaction->MaxOutputFileSize()) {
                    status = FinishCompactionOutputFile(compact, input);
                    if (!status.ok()) {
                        break;
                    }
                }
            }

            input->Next();
        }
        ...
        // 到这一步才把 CompactionStats 中输出文件outputfiles 的记录添加到 VersionEdit 需要的 compaction 中去
        if (status.ok()) {
            status = InstallCompactionResults(compact);
        }
        ...
        return status;
    }
```
上面代码即注释详细描述了合并的过程细节。  
总结一下，就是首先根据 compaction 创建输入文件里的 entries 迭代器 input，然后循环将 input->key() 输出到输出文件 outputfiles 中去。当然，中间是有一些判断条件的，代码中注释讲的挺清楚，不再赘述。

这里还需要提及的一个结构体是 `struct CompactionState`，它封装了一个 `class compaction` 对象，以及输出文件 `vector<outputfile>`，然后还有文件对象 `WritableFile *outfile`，还有 `table` 构建器 `TableBuilder *builder`。详见 `DB::Impl` 中关于该结构体的定义。

`DoCompactionWork()` 中调用的几个函数：  
1. VersionSet::MakeInputIterator()          
2. VersionSet::ShouldStopBefore()            
3. VersionSet::IsBaseLevelForKey()           
4. DB::Impl::OpenCompactionOutputFile()    
5. DB::Impl::FinishCompactionOutputFile()  
6. DB::Impl::InstallCompactionResults()    

第一个是创建迭代器用的函数；二三是versionSet中部分优化函数；  
`OpenCompactionOutputFile()` 用于打开新的文件以及初始化 `table builder`；  
`FinishCompactionOutputFile()` 将写满的文件记录到 `CompactionState` 中，然后置 `outfile` 和 `builder` 为空，等待下次 `OpenCompactionOutputFile()` 调用；  
`InstallCompactionResults()` 把 CompactionStats 中输出文件 outputfiles 的记录添加到 VersionEdit 需要的 compaction 中去。然后函数体内调用 `versions_->LogAndApply()`.

### 总结
`version control` 和 `DB Compaction` 合并操作是两个紧密相连的部件，一个 DB 实例拥有一个 VersionSet 对象（双向链表结构） `versions_` 用于记录每次合并的更改记录。  
而每次合并创建的 `class Compaction` 对象 `compaction`  记录了本次合并的一切文件改动，然后 `version_` 通过调用 `VersionSet::LogAndApply()` 将改动以一个 `version` 对象记录下来。  
`LogAndApply()` 具体做法是先补全 `compaction->edit` 的成员变量，然后创建新的 `version` 节点 `v`，用 `VersionSet::Builder` 来将 `compaction->edit` 的改动记录应用到 `v` 中；接着写入 `MANIFEST` 文件；最后添加 `v` 到 链表 `versions_` 尾部，`versions_->current = v`。