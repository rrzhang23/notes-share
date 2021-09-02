#include "construct.h"


template<typename T>
void print(T& t){
    cout << "T& t" << endl;
}
template<typename T>
void print(const T& t){
    cout << "const T& t" << endl;
}
template<typename T>
void print(T&& t){
    cout << "T&& t" << endl;
}
template<typename T>
void TestForward(T&& v){
    print(std::forward<T>(v));
}


int main(){
    cout << endl << "h" << endl;
    A&& h = A();
    h.Print();                  // data_: nullptr, size_: 0
    h.Set("1111", 4);
    auto&& i = h;               // error when use "A&& i=h;"
    h.Print();                  // data_: 1111, size_: 4
    i.Print();                  // data_: 1111, size_: 4
    cout << addressof(h.data_) << " " << addressof(i.data_) << endl;    // 应该相等


    cout << endl << endl;

    // TestForward(1);
    int const x = 1;
    int y = 1;
    print(x);           // const T& t, 若没有定义 print(const T& t)，只有 print(T& t), 则输出为 T& t
    print(y);           // T& t，若没有定义 print(T& t)，只有 print(const T& t), 则输出为 T&& t
    print(move(x));     // T&& t
    print(move(y));     // T&& t
    print(cref(x));     // T&& t
    print(cref(y));     // T&& t
    // print(std::forward<int>(x));    // error
    print(std::forward<int>(y));    // T&& t

    cout << endl << endl;
    int&& m = 3;
    print(m);           // T& t

    return 0;
}

// g++ move_and_forword.cpp -o move_and_forword.exe