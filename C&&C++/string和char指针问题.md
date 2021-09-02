
关于 `string` 、`char*`、`char[]`问题。 

需要注意两点：
`string` 是经过类包装的数据结构，包含一个 `const char*` 型 `data` ，和一个 `size_t` 类型 `size` 。
`char*` 和 `char[]` 首地址指针都是字符指针。

这三者其实都可以处理字符串中间带 `'\0'` 的情况。只不过 `string` 对于大多函数操作来讲，不会受 `'\0'` 影响；而 `char*` 、 `char[]` 则更容易受影响，包括 `std::cout` 操作。  
所以，复制请选memcpy()函数，而不是strcpy()。初始化string请参照一下方法：  
- 复制请选memcpy()函数  
- 比较请用memcmp()函数

构造方法：  
`std::string` 重载的构造方法很多，包括直接用字面值常量指针等初始化，会从 `'\0'` 处截止，是因为字符指针在赋值前就被截断。  
使用 `string str(size_t, char)` 和 `string::assign(const char*, size_t)` 初始化不受 `'\0'` 影响。  
另外， `const char*`  强制转为 `char*` 是另外开辟了一块空间。

---
```cpp
#include <iostream>
#include <string.h>
using namespace std;

int main(){
    //
    string str1 = "123\0abc8";  //自面值常亮是char* 型，赋给string时就已经在'\0'处截断，只保留了“123”
    char* p1   = (char* )str1.data();   //string::data() 等同于string::c_str()
    cout<<"str1 is : "<<str1<<", and equal to "<<"p1 : "<<p1<<endl;
    cout<<"sizeof str1 : "<<str1.size()<<endl;
    int size1 = 0;   while(*p1){ size1++; p1++; }
    cout<<"sizeof p1 : "<<size1<<endl<<endl;

    string str2(8, 's');
    str2[3] = '\0';
    char* p2 = (char*)str2.c_str();
    cout<<"str2 is : "<<str2<<", and equal to "<<"p2 : "<<p2<<endl;   //此处字符指针只输出了'\0'之前的内容，是因为cout<< 遇到'\0' 就停止了。
    cout<<"sizeof str2 : "<<str2.size()<<endl;
    int size2 = 0;   while(*p2){ size2++; p2++; }
    cout<<"sizeof p2 : "<<size2<<endl<<endl;

    string str3(8, 's');
    str3[3] = '\0';
    char* p3 = (char*)str2.c_str();
    str3.assign(p3,8);
    cout<<"str3 is : "<<str3<<", and equal to "<<"p3 : "<<p3<<endl;
    cout<<"sizeof str3 : "<<str3.size()<<endl;
    int size3 = 0;   while(*p3){ size3++; p3++;}
    cout<<"sizeof p3 : "<<size3<<endl<<endl;

    string str4(8, 's');
    char* p4 = (char*) str4.data();
    int size4 = 0;  while(*p4){size4++; p4++; };
    cout<<"sizeof p4 : "<<size4<<endl<<endl;
}
```