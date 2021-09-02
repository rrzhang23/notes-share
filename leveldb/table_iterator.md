## block 迭代器
先看 block 定义：  
```cpp
class Block {
public:
    uint32_t NumRestarts() const;
    Iterator *NewIterator(const Comparator *comparator){
        return new Iter(cmp, data_, restart_offset_, NumRestarts());
    }
private:
    class Iter;
}
```
这个 `class Iter` 其实是 `class Block` 的内部类。

怎么使用？  
`Block` 内提供了公有函数 `NewIterator()`，其内部调用 `Iter()` 实现迭代器的创建。  
需要传入的参数就是当前 block 的比较器 `cmp`、字符指针 `data_`、`restart point[0]` 的偏移量、重启点个数。  
为什么只需要 `block.data`,不需要 `block.size`?
用 `data[restarts]` 就能得到所有 `entries` 的边界位置，所以不需要 `block.size`。

Iter里面实现了一些基本的迭代器操作，用来实现键值对的读取，具体代码中会有详尽的注释。