### 全局leveldb::Iterator接口： include/leveldb/iterator.h

1. table_block_iterator
2. 
#### 1. table/iterator.cc:
实现了 `leveldb::Iterator` 三个函数：
* `RegisterCleanup()`
* `Iterator()`
* `~Iterator()`

同时派生了一个新类及两个函数：
* `class EmptyIterator`
* `*NewEmptyIterator()`
* `*NewErrorIterator()`

```
    Iterator *NewEmptyIterator() {
        return new EmptyIterator(Status::OK());
    }

    Iterator *NewErrorIterator(const Status &status) {
        return new EmptyIterator(status);
    }
    
```
区别在于传入的参数 `Status` 不同。

***
#### 2. leveldb::DBIter: db/db_iter.h、db/db_iter.cc

***
#### 3. eveldb::SkipList::Iterator: db/skiplist.h

***
#### leveldb::WriteBatch::Iterate db/write_batch.h
这并不是个迭代器，只是个函数，循环处理 `WriteBatch::rep_` 

