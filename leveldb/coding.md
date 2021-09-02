**分定长32位、64位；不定长32/64位。
定长这里不介绍。**

这里无论是key，还是value，存储格式都为:
```
+----------------------+----------------------+
| Slice.size()         | Slice.data()         |
+----------------------+----------------------+
```

### 编码过程:
```cpp
// dst是一个空的char buf[]，32位对应buf[5]( (40-5)bits > 32bits), 64位对应buff[10]( (80-10)bits > 64bits)
// v 是 Slice.size()
EncodeVarint(char *dst, uint32_t v) ;

// 把边长编码后的Slice.size()(即v),以字符形式append到dst后
// dst是一个不确定（或空值）的string, v是Slice.size(), 
PutVarint(std::string *dst, uint32_t v)

// 该函数把 value 拆解成 value.size() + value.data()的形式append到 dst后
// dst是一个不确定（或空值）的string, value使我们要append到dst后的具体数据，
PutLengthPrefixedSlice(std::string *dst, const Slice &value)

编码调用流程：
PutLengthPrefixedSlice() -> PutVarint() -> EncodeVarint()
```
举例：
某 `Slice`  
```cpp

Slice.size() = 1024+512+256+128+64+32+16+8+4+2+1;
```
二进制编码如下：

00000000 00000000 00000<font color=#0000FF>111 1</font><font color=#00FF00>1111111</font>

levelDB编码之后如下：

<font color=#FF0000>1</font><font color=#00FF00>1111111</font> <font color=#FF0000>0</font>000<font color=#0000FF>1111</font>

最终只有 `11111111 00001111`  这两个字节被append到dst中。
`1` 和 `0` 是标志位，解码时碰到 `0` 表示该字节是 `Slice.size()` 的最后一部分。

### 解码过程:

```cpp
// 输入:
//      若input = Slice.len + Slice.data
//      p     指向 input 头
//      limit 指向 input 尾
//      value 未初始化
// 输出:
//      p 向前移动指向 Slice.data, 返回 p
//      value 为 Slice.data 的长度 (uint64_t类型）
    const char *GetVarint64Ptr(const char *p, const char *limit, uint64_t *value) {
        uint64_t result = 0;
        for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
            uint64_t byte = *(reinterpret_cast<const unsigned char *>(p));
            p++;
            if (byte & 128) {
                // More bytes are present
                result |= ((byte & 127) << shift);
            } else {
                result |= (byte << shift);
                *value = result;
                return reinterpret_cast<const char *>(p);
            }
        }
        return nullptr;
    }

// 输入:
//      input 为 Slice.len + Slice.data,
//      value 未初始化。
// 输出:
//      input 向前移动并指向 Slice.data 部分
//      value 为 Slice.data 的长度 (uint64_t类型）
    bool GetVarint64(Slice *input, uint64_t *value) {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = GetVarint64Ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            *input = Slice(q, limit - q);
            return true;
        }
    }

// 输入:
//      input 为 Slice.len + Slice.data,
//      result 未初始化。
// 输出:
//      input 向前移动并指向 Slice.data 部分
//      result 为 Slice.len (Slice类型）
    bool GetLengthPrefixedSlice(Slice *input, Slice *result){
    uint32_t len;
        if (GetVarint32(input, &len) &&
            input->size() >= len) {
            *result = Slice(input->data(), len);
            input->remove_prefix(len);
            return true;
        } else {
            return false;
        }
}

调用流程：
GetLengthPrefixedSlice() -> GetVarint32() -> GetVarint32PtrFallback()
```
假设：
编码后 `Slice *input` 如下：
```cpp
11111111 00001111
```
`GetVarint32PtrFallback()` 中的 `const char *p` 最初应当指向  `11111111` ，然后不断往后++


https://blog.csdn.net/caoshangpa/article/details/78815940