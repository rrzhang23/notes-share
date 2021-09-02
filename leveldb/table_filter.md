>filter 基于 bloomfilter 实现，可参考：  
>[bloomfilter.md](http://note.youdao.com/noteshare?id=e1f54fbeaaff5d0a63abda7c3f720fd3)

### 设计以及实现
#### filter 物理存放格式：
```cpp
                               int_32    int_32    int_32    int_32                  int_8
+---------+---------+---------+---------+---------+---------+-----------------------+--------+
| filter1 | filter2 | filtern | offset1 | offset2 | offsetn | sum(sizeof(filter_i)) | baselg |
+---------+---------+---------+---------+---------+---------+-----------------------+--------+
```
>该格式中 filter_i 对应于 [table.md](http://note.youdao.com/noteshare?id=7ba5a5a566722ffaa27250285b58061c) 中的一个 meta bloc

`leveldb` 采用分段记录 `block` 的过滤信息，`baselg` 默认为11，即针对约 `2^11(2KB)` 的 `data block`，就构建一次 `filter`。  
这些分段的过滤信息就被依次记录到 `filter1` 中。  
`offset_i` 指向 `filter_i` 的偏移量。  
`sum(sizeof(filter_i))` 记录了所有分段 `filter_i` 的长度总和。  

---

采用和 `table_builder`、`block_builder` 相同的设计思想，`filter` 的构建和读取操作也被分离实现。  

#### FilterBlockBuilder
成员变量：  
```cpp
        const FilterPolicy *policy_;    // BloomFilterPolicy
        std::string keys_;              // 用于追加 key，GenerateFilter() 之后清空
        std::vector<size_t> start_;     // 记录 key 在 keys_ 中的位置
        std::string result_;            // 最终生成的字符串，对应文章开始的物理存放格式，用于构建 table
        std::vector<Slice> tmp_keys_;   // 根据 keys_ 和 start_ 构建临时 keys 向量，用于 GenerateFilter() 中调用 policy_->CreateFilter()
        std::vector<uint32_t> filter_offsets_;  // 存放物理组织中 offset_i，即每个 filter_i 的偏移量
```
和 `table_builder`过程结合，得到以下伪代码：
>每个 `table_builder` 拥有一个  `rep_:filter_block(FilterBlockBuilder *)` 对象
* 伪代码：
```
TableBuilder::Add(key) {
    filter_block->AddKey(key) {
        filter_block->start_.push_back(keys_.size());
        filter_block->keys_.append(key.data(), key.size());
        
        if(达到 TableBuilder flush 条件) {   // 此处条件应为一个 block 写满了 4K 左右的空间
            TableBuilder::flush() {
                filter_block->StartBlock(传入 table_builder.file.offset) {
                    定义 filter_index = (table_builder.file.offset / kFilterBase);
                    while (filter_index > filter_block->filter_offsets_.size()) {   // 当 filter_index 大于 offset_i 个数
                        filter_block->GenerateFilter() {
                            构建 tmp_keys_[];
                            filter_block->filter_offsets_.push_back(filter_block->result_.size());
                                CreateFilter(&tmp_keys_[0], static_cast<int>(num_keys), &filter_block->result_);
                            filter_block->tmp_keys_.clear();
                            filter_block->keys_.clear();
                            filter_block->start_.clear();
                        }
        }
                }
            }
        }
    }
TableBuilder::finish() {
    filter_block->Finish() { // 主要构建完整的 result_
        result_ 写入 filter_offsets_[i];
        result_ 写入 sum(sizeof(filter_i));
        result_ 写入 kFilterBaseLg;
    }
    写入 filter_block->result_ 至文件;
}
```
><font color=#FF0000>这个地方有点疑问，根据 filter_builder 描述，应该根据 kFilterBase(约为 2KB)构建meta索引，但是从 table_builder 调用过程来看，好像还是根据 data block 的大小(4KB) 构建的。</font>

#### FilterBlockReader
解析一个由 `FilterBlockBuilder` 构建好的 result_:string。  
结合物理存放格式来看，不难分析。
