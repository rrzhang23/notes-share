#include "construct.h"

int main() {
    A x();                      // 无任何意义，声明一个返回值为 Myclass 对象的无参函数，不调用任何构造函数，仅仅在栈中有对象名，但是没有堆空间
    A y(3);                     // A(int)
    y.Print();                  // data_ 和 size_ 未初始化，可能导致段错误
    A *z = new A(4);            // data_ 和 size_ 均初始化，data_: nullptr, size_: 0
    z->Print();
    A a;                        // A()
    a.Set("1111", 4);
    a.Print();                  // data_: 1111, size_: 4
    
    // 构造函数参数分别是左值和右值
    A b(a);                     // const A&
    a.Print();                  // data_: 1111, size_: 4
    b.Print();                  // data_: nullptr, size_: 0
    A d(ref(a));                // const A&
    a.Print();                  // data_: 1111, size_: 4
    d.Print();                  // data_: nullptr, size_: 0
    A c(std::forward<A>(a));    // A&&
    a.Print();                  // data_: nullptr, size_: 0
    c.Print();                  // data_: 1111, size_: 4
    a.Set("1111", 4);
    A g(std::move(a));          // A&&
    a.Print();                  // data_: nullptr, size_: 0
    g.Print();                  // data_: 1111, size_: 4

    // 使用 operator= 构造
    cout << endl << "e" << endl;
    A e,f;
    a.Set("1111", 4);           
    e = a;                      // operator=(const A&)
    f = std::forward<A>(a);     // operator=(A&&)
    a.Print();                  // data_: nullptr, size_: 0
    f.Print();                  // data_: 1111, size_: 4

    // 临时对象作为构造函数参数，只调用一个构造函数，甚至比右值对象作为参数，函数调用次数更少
    cout << endl << "j" << endl;
    A j(A(2));
    A k = A(2);

    return 0;
}
// g++ construct.cpp -o construct.exe

// 尽量这几种构造方式：
// 1. A a; a.Init();
// 2. A *a = new A();