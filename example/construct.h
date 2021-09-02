#include <iostream>
#include <cstring>
#include <functional>
using namespace std;

namespace {
const char* print_char(char* c) {
  return (c ? c : "nullptr");
}
}

class A{
public:
    char*  data_;
    size_t size_;
    void Init() { size_ = 0; data_ = nullptr; }
    void Set(const char* data, size_t size) {
        this->data_ = new char[size];
        memcpy(this->data_, data, size);
        this->size_ = size;
    }

    A() { Init(); cout << "A()" << endl; }
    A(int a) { cout << "A(int)" << " " << a << endl; }

    A(const A& a) { Init(); cout << "const A&" << endl; }
    A(A&& a) {
        Init();
        std::swap(this->data_, a.data_);
        std::swap(this->size_, a.size_);
        cout << "A&&" << endl;
    }
    A& operator=(const A& a) { Init(); cout << "operator=(const A&)" << endl; return *this;}
    A& operator=(A&& a) {
        Init();
        std::swap(this->data_, a.data_);
        std::swap(this->size_, a.size_);
        cout << "operator=(A&&)" << endl; return *this;
    }

    void Print() { cout << "data_: " << print_char(data_) << ", size_: " << size_ << endl;}
};