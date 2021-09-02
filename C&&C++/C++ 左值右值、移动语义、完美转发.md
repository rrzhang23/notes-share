### 涉及函数及头文件

头文件：`utility`

- std::move
- std::forward

### 左值和右值
[理解 C/C++ 中的左值和右值](https://nettee.github.io/posts/2018/Understanding-lvalues-and-rvalues-in-C-and-C/)

`左值` (lvalue, locator value) 表示了一个占据内存中某个可识别的位置（也就是一个地址）的对象。

`右值` (rvalue) 则使用排除法来定义。一个表达式不是 `左值` 就是 `右值` 。 那么，`右值`是一个“不表示”内存中某个可识别位置的对象的表达式。

例：
~~~cpp
int foo() { return 1; }     // 返回值为右值，若 使用 foo() = 3，则报错，给 “右值” 赋值是错误的。
int& foo() { return 2; }    // 错误，返回值应该是某个左值(即有地址空间的值)，一般用作返回类成员 this 指针。下面是对的：
int globalvar = 20;
int& foo() { return globalvar; }
int main() { foo() = 10; cout << globalvar << ", " << foo() << endl; return 0; }

int var;
var = 4;        // 正确

4 = var;        // 错误！
(var + 1) = 4;  // 错误
~~~

> 右值通常是某个临时变量，或者表达式的中间结果，而没有可识别的内存位置，也就是说，只存在于计算过程中的每个临时寄存器中。

#### 左值引用和右值引用
左值引用的对象必须是一个左值，相对地，右值引用的对象是一个右值。
例：
~~~cpp
class Foo{
    Foo(Foo& f) {}
    Foo(Foo&& f) {}
};
// 分别是左值和右值引用参数，右值引用可以使用这样的表达式初始化：
Foo f(std::move(Foo()));    // 调用 Foo(Foo&& f) {}，因为传入的是一个临时变量
~~~

** CV 限定，即 `const` 和 `volatile `。**

通常左值可以有 CV 限定。而右值，在 C 中是没有 CV 限定的，C++ 中部分有，内置类型没有。

### std::move 和右值引用
使用临时对象初始化某个对象的例子：[test2](../example/construct.cpp)
### 通用引用和右值引用
这两个引用很像，都是使用 `&&` 符号表示，右值引用也是引用，而通用引用则一般作为 std::forward 的形参达到完美转发的目的。

比如：
```
A&& h = A();        // OK，引用右值
auto&& i = h;       // OK，会自动推断右边的类型，并用右值引用引用右边的对象
A&& i=h;            // 错误，此时没有产生自动推断，且右值引用只能引用右值，不能引用左值，h是另一个对象左值
```

右值引用和通用引用有个主要的区别，那就是“类型推断”。在完美转发中，也常常因为形参尽管有着 && 这样的形式，但是没有产生类型推断而没有成功完美转发。
参考：  
[区分通用引用和右值引用](https://www.kancloud.cn/kangdandan/book/169997)

### std::forward 完美转发
即原来是左值，传递后还是左值，原来是右值，传递后还应是右值。
二要素：  
1. 形参一定要是模板参数，且形为 `T&& t`，不能有任何的修饰符，比如 `const T&&`，这是不行的，会导致通用引用失效。
2. 类型推断，一定要产生这个操作，否则也会失效，最后只能按左值传递。

举个例子：[test3](../example/move_and_forward.cpp)

std::vector::push_back，尽管使用了这样的参数形式，但是，由于在声明 vector 对象时，已经造成了特例化，即函数 `push_back，尽管使用了这样的参数形式，但是，由于在声明` 内部知道参数是什么类型的，所以不会进行类型推断，通用引用也自然失效。所以一般推荐emplace_back()。
参考：  
[理解std::move和std::forward](https://www.kancloud.cn/kangdandan/book/169996)