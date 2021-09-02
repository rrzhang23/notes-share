#include <iostream>
#include <cstddef>
#include <string>
#include <cstring>
using namespace std;

class MyString {
public:
    explicit MyString(size_t size = 0) {}
    MyString(char* data, size_t size = 0) : size_(size), data_(data) {}
    MyString(const char* data, size_t size = 0) : size_(size) {
        memcpy(data_, data, size);
    }
// private:
    size_t size_;
    char* data_;
};

int main() {
    char* c1 = new char[3];
    char* c2 = new char[3];
    memset(c1, '0', 3);
    memset(c2, '1', 3);

    MyString str1(c1, 3);
    MyString str2(c2, 3);

    cout << "before swap:" << endl;
    cout << "addof str1: " << addressof(str1) << ", addof str1.data_: "<< addressof(str1.data_) << ", data: " << str1.data_ << endl;
    cout << "addof str2: " << addressof(str2) << ", addof str2.data_: "<< addressof(str2.data_) << ", data: " << str2.data_ << endl;
    swap(str1, str2);
    cout << "after  swap:" << endl;
    cout << "addof str1: " << addressof(str1) << ", addof str1.data_: "<< addressof(str1.data_) << ", data: " << str1.data_ << endl;
    cout << "addof str2: " << addressof(str2) << ", addof str2.data_: "<< addressof(str2.data_) << ", data: " << str2.data_ << endl;

    return 0;
}
// g++ swap.cpp -o swap.exe