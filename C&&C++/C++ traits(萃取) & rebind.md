
~~~cpp
  template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
    class list : protected _List_base<_Tp, _Alloc>
    {
      typedef typename _Alloc::value_type		_Alloc_value_type;  // = _Tp

      typedef _List_base<_Tp, _Alloc>			_Base;
      typedef typename _Base::_Tp_alloc_type		_Tp_alloc_type;
      typedef typename _Base::_Tp_alloc_traits		_Tp_alloc_traits;
      typedef typename _Base::_Node_alloc_type		_Node_alloc_type;
      typedef typename _Base::_Node_alloc_traits	_Node_alloc_traits;

    public:
      typedef _Tp					 value_type;
      typedef typename _Tp_alloc_traits::pointer	 pointer;
      typedef typename _Tp_alloc_traits::const_pointer	 const_pointer;
      typedef typename _Tp_alloc_traits::reference	 reference;
      typedef typename _Tp_alloc_traits::const_reference const_reference;
      typedef _List_iterator<_Tp>			 iterator;
      typedef _List_const_iterator<_Tp>			 const_iterator;
      typedef std::reverse_iterator<const_iterator>	 const_reverse_iterator;
      typedef std::reverse_iterator<iterator>		 reverse_iterator;
      typedef size_t					 size_type;
      typedef ptrdiff_t					 difference_type;
      typedef _Alloc					 allocator_type;

    protected:
      // Note that pointers-to-_Node's can be ctor-converted to
      // iterator types.
      typedef _List_node<_Tp>				 _Node;
    }
~~~

~~~cpp
  template<typename _Tp, typename _Alloc> // 默认 _Alloc = std::allocator<_Tp>
    class _List_base
    {
    protected:
      typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
	rebind<_Tp>::other				_Tp_alloc_type;                                    // _Tp_alloc_type = std::allocator<_Tp>
      typedef __gnu_cxx::__alloc_traits<_Tp_alloc_type>	_Tp_alloc_traits;
      typedef typename _Tp_alloc_traits::template
	rebind<_List_node<_Tp> >::other _Node_alloc_type;                            // _Node_alloc_type = std::allocator<_List_node<_Tp>>
      typedef __gnu_cxx::__alloc_traits<_Node_alloc_type> _Node_alloc_traits;
    }
~~~

~~~cpp
// ext/alloc_traits.h:

template<typename _Alloc>
  struct __alloc_traits
  : std::allocator_traits<_Alloc> // 默认 _Alloc = std::allocator<_Tp>
  {
    typedef std::allocator_traits<_Alloc>           _Base_type;
    template<typename _Tp>
      struct rebind
      { typedef typename _Base_type::template rebind_alloc<_Tp> other; }; // other = std::allocator<_Tp>
      // 即： 默认 _Alloc = std::allocator<_Tp>
      // { typedef typename ::template __alloc_rebind<_Alloc, _Tp> other; };
      // 即： 默认 _Alloc = std::allocator<_Tp>, _Up = _Tp
      // { typedef (typename __allocator_traits_base::template __rebind<_Alloc, _Up>::type) other; }
      // 即：默认 _Alloc = std::allocator<_Tp>
      // { typedef (typename _Alloc::template rebind<_Tp>::other) other; }
      // 即：
      // { typedef std::allocator<_Tp> other; }
  }
~~~

~~~cpp
// bits/alloc_traits.h:

  template<typename _Alloc> // 默认 _Alloc = std::allocator<_Tp>
  struct allocator_traits : __allocator_traits_base
  {
    /// The allocator type
    typedef _Alloc allocator_type;
    /// The allocated type
    typedef typename _Alloc::value_type value_type; // 默认 value_type = _Tp

    
      template<typename _Tp>
	using rebind_alloc = __alloc_rebind<_Alloc, _Tp>;
      template<typename _Tp>
	using rebind_traits = allocator_traits<rebind_alloc<_Tp>>;
  }

  struct __allocator_traits_base
  {
    template<typename _Tp, typename _Up, typename = void>
      struct __rebind : __replace_first_arg<_Tp, _Up> { };
    // 特化版本，此处 _Tp = std::alloctor<_Up>，而 std::alloctor 是实现了 rebind 的
    template<typename _Tp, typename _Up>
      struct __rebind<_Tp, _Up,
		      __void_t<typename _Tp::template rebind<_Up>::other>>
      { using type = typename _Tp::template rebind<_Up>::other; };
  };

  template<typename _Alloc, typename _Up>
    using __alloc_rebind
      = typename __allocator_traits_base::template __rebind<_Alloc, _Up>::type;
~~~

~~~cpp
  template<typename _Tp>
    class new_allocator
    {
    public:
      template<typename _Tp1>
	struct rebind
	{ typedef new_allocator<_Tp1> other; };
    }
~~~