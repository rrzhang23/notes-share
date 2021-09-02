# Vector
C++ STL 顺序容器之一，之前一直在使用，但是搞不明白和 std::array 的区别，比如 array 是定长数组，vector 动态容器，是如何做到动态增长的？    
迭代器 Iterator 原理是什么？  
vector 存的是指针还是值？iter 和 operator[]、at 返回值类型？  
`push_back` 和 `emplace_back` 区别？
`reserve` 和 `resize` 区别？

## 简介

vector 内部是一个连续分配的空间，假定实例化一个 vector<T> 类型的动态数组，那么里面存放的就是一个个 T 类型对象；若是实例化  vector<T*> ，那么存放的就是指针。  
若数组长度为 10，那么 vector<bool> 就会占用 10*sizeof(bool)（约为 10 bytes） 大小的连续空间，vector<bool*> 占用 10*sizeof(bool*)（约为 80 bytes）的连续空间。  
尽管内部是连续空间存储元素，但是由于 vector 的动态特性，在下一个元素进来之后，vector 内部没有足够空间时，会先申请一块更大的空间，把之前的元素复制进来，再追加元素。这就是 vector 扩容可能使迭代器失效的原因。


## 内部实现
vector 继承自 _Vector_base
```cpp
// stl_vector.h:
class vector : protected _Vector_base<_Tp, _Alloc>
```
`_Vector_base` 内部定义了 `struct _Vector_impl` 来管理几个指针，用于指向元素的数据，分别是：
~~~cpp
      struct _Vector_impl
      : public _Tp_alloc_type
      {
	    pointer _M_start;           // 指向连续空间的首地址
	    pointer _M_finish;          // 指向最后一个元素后的地址，若是元素占满了已分配的连续空间，则 _M_finish 指向的地址未分配(即等于 _M_end_of_storage)
	    pointer _M_end_of_storage;  // 指向连续空间后的地址
      }
~~~

而 vector 则管理了一个 struct _Vector_impl 对象：
~~~cpp
class vector : protected _Vector_base<_Tp, _Alloc> {
public:
    _Vector_impl _M_impl;
}
~~~


### `reserve` 和 `resize`
~~~cpp
// stl_vector.h:
    void resize(size_type __new_size)
    {
        if (__new_size > size())
        _M_default_append(__new_size - size());                 // _M_end_of_storage - _M_start 判断空间不够时，会重新分配空间，并修改 _Vector_impl 内的三个指针位置 
        else if (__new_size < size())
        _M_erase_at_end(this->_M_impl._M_start + __new_size);   // 直接调用 std::_Destroy 销毁（析构） (_M_start+__new_size, _M_finish) 之间的所有元素，但是不会释放内存，因为不会改变 _M_end_of_storage 指向
    }
// vector.tcc
    reserve(size_type __n)
    {
        if (this->capacity() < __n)
	    {
	    const size_type __old_size = size();
	    pointer __tmp = _M_allocate_and_copy(__n,
	            _GLIBCXX_MAKE_MOVE_IF_NOEXCEPT_ITERATOR(this->_M_impl._M_start),
	            _GLIBCXX_MAKE_MOVE_IF_NOEXCEPT_ITERATOR(this->_M_impl._M_finish));
	    std::_Destroy(this->_M_impl._M_start, this->_M_impl._M_finish,
			    _M_get_Tp_allocator());
	    _M_deallocate(this->_M_impl._M_start,
			    this->_M_impl._M_end_of_storage
			    - this->_M_impl._M_start);
	    this->_M_impl._M_start = __tmp;
	    this->_M_impl._M_finish = __tmp + __old_size;
	    this->_M_impl._M_end_of_storage = this->_M_impl._M_start + __n;
	    }
    }
~~~

共同点：  
1. 原来的空间不够时，都会重新申请内存，并复制元素，然后析构之前的内存，并释放原来内存。  
2. 原来空间够时，resize 可能会调用析构来销毁多余的元素；reserve 则什么也不做。

### Iterator、[]、at
#### Iterator
~~~cpp
// stl_vector.h:
    typedef typename __gnu_cxx::__alloc_traits<_Tp_alloc_type>::pointer pointer;
    struct _Vector_impl
    : public _Tp_alloc_type
    {
        pointer _M_start;           // 指向连续空间的首地址
        pointer _M_finish;          // 指向最后一个元素后的地址，若是元素占满了已分配的连续空间，则 _M_finish 指向的地址未分配(即等于 _M_end_of_storage)
        pointer _M_end_of_storage;  // 指向连续空间后的地址
    }

    typedef __gnu_cxx::__normal_iterator<pointer, vector> iterator;
    iterator begin() _GLIBCXX_NOEXCEPT
    { return iterator(this->_M_impl._M_start); }
    iterator end() _GLIBCXX_NOEXCEPT
    { return iterator(this->_M_impl._M_finish); }
~~~

迭代器返回的实际上是元素的指针，begin() 则是 _M_start，end() 则是 _M_finish（end()-1 才是最后一个元素）。  
> 至于 pointer 和 iterator 类型为什么不一致，还要看具体代码怎么实现的...

#### []
~~~cpp
// stl_vector.h:
typedef typename _Alloc_traits::reference
reference operator[](size_type __n) _GLIBCXX_NOEXCEPT
      { __glibcxx_requires_subscript(__n); return *(this->_M_impl._M_start + __n); }
~~~


#### at
~~~cpp
// stl_vector.h:
typedef typename _Alloc_traits::reference
reference at(size_type __n) {
	_M_range_check(__n); return (*this)[__n]; }
~~~

### `push_back` 和 `emplace_back`
动态增长：
~~~cpp
// vector.tcc
    emplace_back(_Args&&... __args)
    {
        if (this->_M_impl._M_finish != this->_M_impl._M_end_of_storage)
        {
            _Alloc_traits::construct(this->_M_impl, this->_M_impl._M_finish,
                    std::forward<_Args>(__args)...);
            ++this->_M_impl._M_finish;
        }
    else
        _M_realloc_insert(end(), std::forward<_Args>(__args)...);
    }


    _M_realloc_insert(iterator __position, const _Tp& __x)
    {
    const size_type __len = _M_check_len(size_type(1), "vector::_M_realloc_insert");    // 检查是否超过 max_size()（该值由平台决定），并返回需要重新分配的大小（元素个数）
      const size_type __elems_before = __position - begin();    // 之前元素个数（为什么不直接调用 size()?）
      pointer __new_start(this->_M_allocate(__len));
      pointer __new_finish(__new_start);
      __try
	{
	  // The order of the three operations is dictated by the C++11
	  // case, where the moves could alter a new element belonging
	  // to the existing vector.  This is an issue only for callers
	  // taking the element by lvalue ref (see last bullet of C++11
	  // [res.on.arguments]).
      // 新内存位置适当构造，以存放push进来的元素
	  _Alloc_traits::construct(this->_M_impl, __new_start + __elems_before,
#if __cplusplus >= 201103L
				   std::forward<_Args>(__args)...);
#else
				   __x);
#endif
	  __new_finish = pointer();

	  __new_finish
	    = std::__uninitialized_move_if_noexcept_a
	    (this->_M_impl._M_start, __position.base(),
	     __new_start, _M_get_Tp_allocator());           // 移动之前内存数据到新的内存

	  ++__new_finish;

	  __new_finish
	    = std::__uninitialized_move_if_noexcept_a
	    (__position.base(), this->_M_impl._M_finish,
	     __new_finish, _M_get_Tp_allocator());
	}
    __catch(...) { ... }
    std::_Destroy(this->_M_impl._M_start, this->_M_impl._M_finish,  _M_get_Tp_allocator());     // 析构之前内存的元素，不释放内存
    _M_deallocate(this->_M_impl._M_start, this->_M_impl._M_end_of_storage - this->_M_impl._M_start); // 这一步才释放
    // 内部指针指向新的空间：
    this->_M_impl._M_start = __new_start;
    this->_M_impl._M_finish = __new_finish;
    this->_M_impl._M_end_of_storage = __new_start + __len;
    }
~~~

`push_back` 和 `emplace_back` 区别:
