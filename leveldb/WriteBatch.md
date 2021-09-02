WriteBatch包含一个成员 rep_:string, 内部组织：
```
  8 bytes           4 bytes                      
+------------+---------+-------------+-------------+
| sequence   | count   | operation   | operation   |
+------------+---------+-------------+-------------+
```

operation 如下：     
```
+-------------+--------------------------+-------------------+-----------------------+----------------+
| type:enum   | user_key.size():size_t   | user_key.data()   | value.size():size_t   | value.data()   |
+-------------+--------------------------+-------------------+-----------------------+----------------+
```

sequence 仅仅在 Iterate() 才使用

```
enum ValueType {
        kTypeDeletion = 0x0,
        kTypeValue = 0x1
    };
```

```
 class LEVELDB_EXPORT WriteBatch {
    public:

        void Put(const Slice &key, const Slice &value);
        void Delete(const Slice &key);
        void Clear();
        size_t ApproximateSize() const;
        void Append(const WriteBatch &source);

        // Handler由MemTableInserter继承，具体格式见最后class MemTableInserter
        class Handler {
        public:
            virtual ~Handler();
            virtual void Put(const Slice &key, const Slice &value) = 0;
            virtual void Delete(const Slice &key) = 0;
        };

// 通过Handler 句柄，把 rep_中的操作循环写入到 MemTableInserter 的成员 MemTable当中去。
        Status Iterate(Handler *handler) const;

    private:
        friend class WriteBatchInternal;
        std::string rep_;  // See comment in write_batch.cc for the format of rep_
    }
```

```
class MemTableInserter : public WriteBatch::Handler {
        public:
            SequenceNumber sequence_;
            MemTable *mem_;

            virtual void Put(const Slice &key, const Slice &value) {
                mem_->Add(sequence_, kTypeValue, key, value);
                sequence_++;
            }

            virtual void Delete(const Slice &key) {
                mem_->Add(sequence_, kTypeDeletion, key, Slice());
                sequence_++;
            }
        };
```
