

参考：[C++标准库（五）之智能指针源码剖析](https://www.cnblogs.com/ukernel/p/9191157.html)
# 涉及的几个类
`shared_ptr<_Tp>`, `__shared_ptr<_Tp, _Lock_policy>`, `__shared_ptr_access<_Tp, _Lock_policy>`, `_Sp_counted_base<_Lock_policy>`, `__shared_count<_Lock_policy>`

`_Sp_counted_ptr<_Ptr, _Lock_policy>`, `_Sp_counted_deleter<_Ptr, _Deleter, _Alloc, _Lock_policy>`, `_Sp_counted_ptr_inplace<_Ptr, _Alloc, _Lock_policy>`

`_Mutex_base<_Lock_policy>`
# 继承关系及主要功能
`shared_ptr` -> `__shared_ptr` -> `__shared_ptr_access`

`_Sp_counted_ptr`、`_Sp_counted_deleter`、`_Sp_counted_ptr_inplace` -> `_Sp_counted_base`

### shared_ptr：
~~~cpp
// experimental/bits/shared_ptr.h
class shared_ptr : public __shared_ptr<_Tp>
{
    using _Base_type = __shared_ptr<_Tp>;
	
    shared_ptr(_Tp1* __p, _Deleter __d, _Alloc __a) : _Base_type(__p, __d, __a) { _M_enable_shared_from_this_with(__p); }
    ...

    shared_ptr(nullptr_t __p, _Deleter __d, _Alloc __a) : _Base_type(__p, __d, __a) { }
    ...

    shared_ptr(const shared_ptr<_Tp1>& __r, element_type* __p) noexcept
	: _Base_type(__r, __p) { }
    ...

	operator=(shared_ptr<_Tp1>&& __r) noexcept
	operator=(std::auto_ptr<_Tp1>&& __r)
	operator=(unique_ptr<_Tp1, _Del>&& __r);
    ...
};
~~~
shared_ptr 继承自 __shared_ptr，内部除了多种构造函数，没有什么特别的。`get`、`reset`、`use_count`、`swap`、`unique`、`onwer_before` 都是由 __shared_ptr 实现的。这里拷贝构造函数确实会引起计数器变化，但是实现不在这里。

### __shared_ptr：
~~~cpp
// bits/shared_ptr_base.h
class __shared_ptr
    : public __shared_ptr_access<_Tp, _Lp>
{
private:
    element_type*	   _M_ptr;           // Contained pointer. 被管理的元素指针
    __shared_count<_Lp>  _M_refcount;    // Reference counter. 计数器

    constexpr __shared_ptr() noexcept : _M_ptr(0), _M_refcount() { }
    explicit __shared_ptr(_Yp* __p) : _M_ptr(__p), _M_refcount(__p, typename is_array<_Tp>::type()) {
	  _M_enable_shared_from_this_with(__p);
	}
    __shared_ptr(const __shared_ptr&) noexcept = default;
    __shared_ptr& operator=(const __shared_ptr&) noexcept = default;
    ~__shared_ptr() = default;
    __shared_ptr(__shared_ptr&& __r) noexcept : _M_ptr(__r._M_ptr), _M_refcount() {
	    _M_refcount._M_swap(__r._M_refcount);
	    __r._M_ptr = 0;
    }
    ... // 其他构造函数

	operator=(const __shared_ptr<_Yp, _Lp>& __r) noexcept {
	  _M_ptr = __r._M_ptr;
	  _M_refcount = __r._M_refcount; // __shared_count::op= doesn't throw
	  return *this;
	}
	operator=(auto_ptr<_Yp>&& __r) {
	  __shared_ptr(std::move(__r)).swap(*this);
	  return *this;
	}

    operator=(__shared_ptr&& __r) noexcept {
	    __shared_ptr(std::move(__r)).swap(*this);
	    return *this;
    }
	operator=(__shared_ptr<_Yp, _Lp>&& __r) noexcept {
	  __shared_ptr(std::move(__r)).swap(*this);
	  return *this;
	}
	operator=(unique_ptr<_Yp, _Del>&& __r) {
	    __shared_ptr(std::move(__r)).swap(*this);
	    return *this;
	}

    void reset() noexcept { __shared_ptr().swap(*this); }
	reset(_Yp* __p) // _Yp must be complete. {
	  // Catch self-reset errors.
	  __glibcxx_assert(__p == 0 || __p != _M_ptr);
	  __shared_ptr(__p).swap(*this);
	}
	reset(_Yp* __p, _Deleter __d) { __shared_ptr(__p, std::move(__d)).swap(*this); }
    reset(_Yp* __p, _Deleter __d, _Alloc __a) { __shared_ptr(__p, std::move(__d), std::move(__a)).swap(*this); }

    element_type* get() const noexcept { return _M_ptr; }
    explicit operator bool() const { return _M_ptr == 0 ? false : true; }
    bool unique() const noexcept { return _M_refcount._M_unique(); }
    long use_count() const noexcept { return _M_refcount._M_get_use_count(); }
    void swap(__shared_ptr<_Tp, _Lp>& __other) noexcept {
	    std::swap(_M_ptr, __other._M_ptr);
	    _M_refcount._M_swap(__other._M_refcount);
    }
	bool owner_before(__shared_ptr<_Tp1, _Lp> const& __rhs) const noexcept { return _M_refcount._M_less(__rhs._M_refcount); }
	bool owner_before(__weak_ptr<_Tp1, _Lp> const& __rhs) const noexcept { return _M_refcount._M_less(__rhs._M_refcount); }

};
~~~
从上面几个函数可以看出几点：
1. 没有提供 shared_ptr = new _Tp() 的构造函数，operator= 操作符也仅限于几种智能指针（auto_ptr、unique_ptr、shared_ptr 等）。
2. operator= 中确实可以看出，多个 shared_ptr 指向了同一个原始指针，但是这些 shared_ptr 内部计数器 __shared_count 是独占的，具体是如何增减的，在 __shared_ptr 中不能提现，要看 __shared_count 源码。
3. reset 采用新建对象，然后 swap 的方式：`__shared_ptr().swap(*this);` 之前的 shared_ptr 可能正在被其他地方引用，所以不能直接置空，析构函数等其他引用结束后，根据内部 __shared_count 对象决定是否析构。
4. unique、use_count 需要由 _M_refcount 给出。

### __shared_count:
~~~cpp
template<typename _Ptr, _Lock_policy _Lp>
class _Sp_counted_ptr final : public _Sp_counted_base<_Lp>
{
public:
    explicit  _Sp_counted_ptr(_Ptr __p) noexcept : _M_ptr(__p) { }
    virtual void _M_dispose() noexcept { delete _M_ptr; }   // delete 原始指针
    virtual void _M_destroy() noexcept { delete this; }     // delete 当前计数器
    virtual void* _M_get_deleter(const std::type_info&) noexcept { return nullptr; }

    _Sp_counted_ptr(const _Sp_counted_ptr&) = delete;
    _Sp_counted_ptr& operator=(const _Sp_counted_ptr&) = delete;
private:
    _Ptr             _M_ptr;
};

template<_Lock_policy _Lp>
class __shared_count
{
private:
    friend class __weak_count<_Lp>;
    _Sp_counted_base<_Lp>*  _M_pi;

public:
    constexpr __shared_count() noexcept : _M_pi(0)  { }

    template<typename _Ptr>
    explicit __shared_count(_Ptr __p) : _M_pi(0) {
	    _M_pi = new _Sp_counted_ptr<_Ptr, _Lp>(__p);    // _Sp_counted_ptr 继承自 _Sp_counted_base，记录了原始指针信息
	}

    // Throw bad_weak_ptr when __r._M_get_use_count() == 0.
    explicit __shared_count(const __weak_count<_Lp>& __r);
    // Does not throw if __r._M_get_use_count() == 0, caller must check.
    explicit __shared_count(const __weak_count<_Lp>& __r, std::nothrow_t);
    ~__shared_count() noexcept {
        if (_M_pi != nullptr)
        _M_pi->_M_release();
    }

    __shared_count(const __shared_count& __r) noexcept : _M_pi(__r._M_pi) {
        if (_M_pi != 0)
            _M_pi->_M_add_ref_copy();
    }
    __shared_count& operator=(const __shared_count& __r) noexcept {
        _Sp_counted_base<_Lp>* __tmp = __r._M_pi;
        if (__tmp != _M_pi) {
            if (__tmp != 0)
                __tmp->_M_add_ref_copy();
            if (_M_pi != 0)
                _M_pi->_M_release();
            _M_pi = __tmp;
        } // else __tmp == _M_pi，说明该函数调用前，this 已经是一个原始指针的计数器，那么不需要任何计数器加或减的操作
        return *this;
    }

    

    long _M_get_use_count() const noexcept { return _M_pi != 0 ? _M_pi->_M_get_use_count() : 0; }
    bool _M_unique() const noexcept { return this->_M_get_use_count() == 1; }
    void* _M_get_deleter(const std::type_info& __ti) const noexcept { return _M_pi ? _M_pi->_M_get_deleter(__ti) : nullptr; }
    friend inline bool operator==(const __shared_count& __a, const __shared_count& __b) noexcept { return __a._M_pi == __b._M_pi; }
};
~~~
1. 计数器是基类 _Sp_counted_base，_Sp_counted_ptr 比基类多了原始指针的信息，托管原始指针时，计数器对象才会被 new 出来，否则置0(nullptr)。
2. 每个 __shared_ptr 拥有独立的 __shared_count 对象，但是这些 __shared_count 中的 _Sp_counted_base 指向的是同一个计数对象，这个计数对象的增减在内部完成。

### _Sp_counted_base:
~~~cpp
template<_Lock_policy _Lp = __default_lock_policy>
class _Sp_counted_base : public _Mutex_base<_Lp>
{
public:
    _Sp_counted_base() noexcept : _M_use_count(1), _M_weak_count(1) { }

    virtual ~_Sp_counted_base() noexcept { }

    // Called when _M_use_count drops to zero, to release the resources
    // managed by *this.
    virtual void _M_dispose() noexcept = 0;     // delete 原始指针
    // Called when _M_weak_count drops to zero.
    virtual void _M_destroy() noexcept { delete this; } // delete 当前计数器
    virtual void* _M_get_deleter(const std::type_info&) noexcept = 0;

    void _M_add_ref_copy() { __gnu_cxx::__atomic_add_dispatch(&_M_use_count, 1); }
    void _M_add_ref_lock();
    bool _M_add_ref_lock_nothrow();

    void _M_release() noexcept {
        // Be race-detector-friendly.  For more info see bits/c++config.
        _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_use_count);
        if (__gnu_cxx::__exchange_and_add_dispatch(&_M_use_count, -1) == 1) {
                _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_use_count);
                _M_dispose();
            // There must be a memory barrier between dispose() and destroy()
            // to ensure that the effects of dispose() are observed in the
            // thread that runs destroy().
            // See http://gcc.gnu.org/ml/libstdc++/2005-11/msg00136.html
            if (_Mutex_base<_Lp>::_S_need_barriers) {
                __atomic_thread_fence (__ATOMIC_ACQ_REL);
            }

                // Be race-detector-friendly.  For more info see bits/c++config.
                _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_weak_count);
            if (__gnu_cxx::__exchange_and_add_dispatch(&_M_weak_count,
                                    -1) == 1) {
                    _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_weak_count);
                    _M_destroy();
            }
        }
    }

    void _M_weak_add_ref() noexcept { __gnu_cxx::__atomic_add_dispatch(&_M_weak_count, 1); }

    void _M_weak_release() noexcept {
    // Be race-detector-friendly. For more info see bits/c++config.
        _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_weak_count);
        if (__gnu_cxx::__exchange_and_add_dispatch(&_M_weak_count, -1) == 1) {
            _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_weak_count);
            if (_Mutex_base<_Lp>::_S_need_barriers) {
                // See _M_release(),
                // destroy() must observe results of dispose()
                    __atomic_thread_fence (__ATOMIC_ACQ_REL);
            }
            _M_destroy();
        }
    }

    long _M_get_use_count() const noexcept {
        // No memory barrier is used here so there is no synchronization
        // with other threads.
        return __atomic_load_n(&_M_use_count, __ATOMIC_RELAXED);
    }

private:
    _Sp_counted_base(_Sp_counted_base const&) = delete;
    _Sp_counted_base& operator=(_Sp_counted_base const&) = delete;
    // _Atomic_word = int
    _Atomic_word  _M_use_count;     // #shared
    _Atomic_word  _M_weak_count;    // #weak + (#shared != 0)
};
~~~
1. _Sp_counted_base 通常不直接使用，而是在托管原始指针时，__shared_count::_M_pi = new _Sp_counted_ptr<_Ptr, _Lp>(__p); 因为计数器需要在计数为 0 时调用原始指针的析构函数，而 _Sp_counted_ptr 知道原始指针的信息。
2. 初始化时，_M_use_count 之所以为 1，是因为 shared_ptr 没有托管任何原始指针时，仅仅声明（新建）一个对象是不会调用 new _Sp_counted_base() 的，只有第一次托管原始指针，_Sp_counted_base 才会被 new。
3. 两个最常用的函数 `_M_add_ref_copy`、`_M_release` 仅仅是对值 _M_use_count 进行加减操作，_M_release 会判断是否是最后一次调用，是的话会通过 `_M_dispose` 调用原始指针的析构函数。
4. TODO: __atomic_thread_fence 内存栅栏问题。


## 总结
1. 每个 shared_ptr 都包含了一个 __shared_count 对象，而每个 __shared_count 包含了一个 `_Sp_counted_base` 指针，当托管的原始指针相同时，这些 __shared_count 的 _Sp_counted_base 都指向同一块空间。最终由 _Sp_counted_base 来执行计数。  
2. _Sp_counted_base 默认通过原子操作完成，是线程安全的，但是共享指针包含了两个操作，一个是原始指针的指向，另一个是计数器的累加/减，这两部操作并不是同步的。所以线程安全仍然需要考虑。参考：[为什么多线程读写 shared_ptr 要加锁？ - 陈硕](https://blog.csdn.net/solstice/article/details/8547547)

## 初始化
1. 仅声明一个共享指针对象：  
shared_ptr<int> a; 指向的原始指针和 __shared_count 对象都会初始化：`: _M_ptr(0), _M_refcount()`，_M_ptr 指向空指针，而 _M_refcount 则进一步调用 __shared_count 构造函数，__shared_count 内部指针对象 `_Sp_counted_base<_Lp>*  _M_pi` 会初始化为空指针 `__shared_count() noexcept : _M_pi(0) {}`。
2. 声明时传入原始指针地址：  
shared_ptr<int> a(new int(1)); `__shared_ptr(_Yp* __p) : _M_ptr(__p), _M_refcount(__p...) {}` 原始指针指向新new出的对象，而 _M_refcount 内部则会 new _Sp_counted_base 对象：`_M_pi = new _Sp_counted_ptr<_Ptr, _Lp>(__p);`
3. 拷贝构造/复制构造：  
operator=、__shared_count(const __shared_count&)、__shared_count(__shared_count&&)。前面两个没有重载，都是默认调用拷贝函数，即 _M_ptr 简单指向另一个 shared_ptr._M_ptr，而 shared_ptr._M_refcount 会调用 `__shared_count(const __shared_count&)`，这个函数内部如下：  
~~~cpp
    __shared_count(const __shared_count& __r) noexcept : _M_pi(__r._M_pi) {
	    if (_M_pi != 0)
	    _M_pi->_M_add_ref_copy();
    }
~~~
直接将 _Sp_counted_ptr 指向另一个 _Sp_counted_ptr，同时引用计数器加 1。

而移动构造 `__shared_count(__shared_count&&)`：  
__shared_ptr 直接调用 __shared_count::swap 交换掉属性 _M_refcount，临时右值的 _M_ptr 置空，且右值的 _M_refcount 被置换为空指针状态。
~~~cpp
__shared_ptr::__shared_ptr(__shared_ptr&& __r) noexcept : _M_ptr(__r._M_ptr), _M_refcount() {
	_M_refcount._M_swap(__r._M_refcount);
	__r._M_ptr = 0;
}

void __shared_count::_M_swap(__shared_count& __r) noexcept {
    _Sp_counted_base<_Lp>* __tmp = __r._M_pi;
    __r._M_pi = _M_pi;
    _M_pi = __tmp;
}
~~~

4. 从 unique_ptr、auto_ptr等右值构造（注意，是右值。且不包含 weak_ptr）。

## operator=
1. 参数为左值，此时调用者可能为 为指向任何指针、指向不同的原始指针、指向和参数相同的原始指针，所以采用 `_M_refcount = __r._M_refcount;`，这行代码会调用 __shared_count::operator=, 这个函数会判断上述三种情况。
2. 参数为右值，直接新建一个 __shared_ptr，然后调用 __shared_ptr::swap，内部也是调用 __shared_count::_M_swap。


# weak_ptr
1. weak_ptr 主要为了配合 shared_ptr 使用，避免循环引用。weak_ptr 不能直接托管原始指针，但是可以把 shared_ptr 复制给 weak_ptr，此时虽然二者本身的属性 __*_count 不一样，但是最终指向的都是同一个 _Sp_counted_base 计数器对象。
2. lock() 返回的是新构造的 shared_ptr，新的 shared_ptr 内部计数器指针实际上指向的是 weak_ptr::__weak_count::_Sp_counted_base，同时 _Sp_counted_base._M_use_count 会加1（存在原始指针的情况下，否则计数器为空指针）。
3. 计数器对象的新建仅仅和原始指针绑定，即只会在 __shared_count 中被创建，__weak_count 是不会新建的。
4. 一旦原始指针存在，weak_ptr 监视了之后，lock() 才会增加计数器 _M_use_count，weak_ptr 对象再多、再怎么复制也只是增加 _M_weak_count。
5. 计数器 _Sp_counted_base 的生命周期由 _M_weak_count 决定，_M_weak_count 始终等于 weak_ptr 数量减去1，因为第一个 shared_ptr 新建 _Sp_counted_base 时，将 _M_weak_count 赋值为 1。最后一个 weak_ptr 或者 shared_ptr 析构时，会检测到 _Sp_counted_base._M_weak_count==0，然后析构计数器本身。


# 函数释义
> // 第一个参数加上一个值，并返回第一个参数的旧值   
> _Atomic_word __exchange_and_add_dispatch(volatile _Atomic_word*, int);  
> // 第一个参数加上一个值  
> void __atomic_add_dispatch(volatile _Atomic_word*, int);
> 
>
> __atomic_thread_fence

> 参考 ：[Chapter 29. Concurrency](https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_concurrency.html)

# auto_ptr
没有计数器，仅仅在内部定义了一个指针 `_M_ptr` 指向原始指针，在 auto_ptr 对象析构时，自动调用 `delete _M_ptr`。