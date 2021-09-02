## const修饰函数 
在类中将成员函数修饰为const表明在该函数体内，<font color=#FF0000 >不能修改对象的数据成员而且不能调用非const函数。</font>  
为什么不能调用非const函数？因为非const函数可能修改数据成员，const成员函数是不能修改数据成员的，所以在const成员函数内只能调用const函数。