### 涉及函数及头文件

头文件：`functional`

- std::function
- std::bind
- lambda

### C++中的各种可调用对象
[C++中的各种可调用对象](https://www.cnblogs.com/youyoui/p/8933006.html)

- 普通函数
- 类成员函数
- 类静态函数
- 仿函数
- 函数指针
- lambda表达式 C++11加入标准
- std::function C++11加入标准

#### 普通函数

#### 类的函数
调用方法：  
一般函数通过obj.Foo() 方式调用，  
而静态函数则通过 A::Foo() 方式调用。 

#### 仿函数

即重载了 operator()，可以直接通过 obj() 方式调用，而不是obj.foo() 。

#### 函数指针
例:
~~~cpp
int (*compute)(int, int);

int max(int x, int y) { return x >= y ? x : y; }
int min(int x, int y) { return x <= y ? x : y; }
int add(int x, int y) { return x + y; }
int multiply(int x, int y) { return x * y; }

int main(void) 
{
	int x = 2; 
	int y = 5;
	std::cout << "max: " << compute_x_y(x, y, max) << std::endl; // max: 5
	std::cout << "min: " << compute_x_y(x, y, min) << std::endl; // min: 2
	std::cout << "add: " << compute_x_y(x, y, add) << std::endl; // add: 7
	std::cout << "multiply: " << compute_x_y(x, y, multiply) << std::endl; // multiply: 10
	
	// 无捕获的lambda可以转换为同类型的函数指针
	auto sum = [](int x, int y)->int{ return x + y; };
	std::cout << "sum: " << compute_x_y(x, y, sum) << std::endl; // sum: 7
	
    getchar();
    return 0;
}
~~~

通常配合回调函数使用。

#### Lambda函数
形式：  
[captures] (params) -> return_type { statments;} 

> 捕获列表和参数列表有区别。


### std::function
在C++11后加入标准，可以用它来描述C++中所有可调用实体，它是是可调用对象的包装器，声明如下：
~~~cpp
#include <functional>

// 声明一个返回值为int，参数为两个int的可调用对象类型
std::function<int(int, int)> Func;
~~~

可以类似函数指针，配合回调函数使用。

#### 其他函数实体转化为std::function
~~~cpp
#include <iostream>
#include <functional>

std::function<int(int, int)> SumFunction;

// 普通函数
int func_sum(int a, int b) { return a + b; }

class Calcu {
public:
	int base = 20;
	// 类的成员方法，参数包含this指针
	int class_func_sum(const int a, const int b) const { return this->base + a + b; };
	// 类的静态成员方法，不包含this指针
	static int class_static_func_sum(const int a, const int b) { return a + b; };
};

// 仿函数
class ImitateAdd {
public:
	int operator()(const int a, const int b) const { return a + b; };
};

// lambda函数
auto lambda_func_sum = [](int a, int b) -> int { return a + b; };

// 函数指针
int (*func_pointer)(int, int);

int main(void) 
{
	int x = 2; 
	int y = 5;

	// 普通函数
	SumFunction = func_sum;
	int sum = SumFunction(x, y);
	std::cout << "func_sum：" << sum << std::endl;

	// 类成员函数
	Calcu obj;
	SumFunction = std::bind(&Calcu::class_func_sum, obj, 
		std::placeholders::_1, std::placeholders::_2); // 绑定this对象
	sum = SumFunction(x, y);
	std::cout << "Calcu::class_func_sum：" << sum << std::endl;

	// 类静态函数
	SumFunction = Calcu::class_static_func_sum;
	sum = SumFunction(x, y);
	std::cout << "Calcu::class_static_func_sum：" << sum << std::endl;

	// lambda函数
	SumFunction = lambda_func_sum;
	sum = SumFunction(x, y);
	std::cout << "lambda_func_sum：" << sum << std::endl;

	// 带捕获的lambda函数
	int base = 10;
	auto lambda_func_with_capture_sum = [&base](int x, int y)->int { return x + y + base; };
	SumFunction = lambda_func_with_capture_sum;
	sum = SumFunction(x, y);
	std::cout << "lambda_func_with_capture_sum：" << sum << std::endl;

	// 仿函数
	ImitateAdd imitate;
	SumFunction = imitate;
	sum = SumFunction(x, y);
	std::cout << "imitate func：" << sum << std::endl;

	// 函数指针
	func_pointer = func_sum;
	SumFunction = func_pointer;
	sum = SumFunction(x, y);
	std::cout << "function pointer：" << sum << std::endl;

	getchar();
	return 0;
}
~~~


### std::bind、std::ref、std::cref、std::mem_fn
后几个通常和 bind 结合使用。  
[example](../example/functional.cpp)

> std::bind 总是使用值拷贝的形式传参，哪怕函数声明为引用  
> std::bind 可以使用std::ref来传引用  
> std::bind 虽然使用了std::ref传递了引用，如果函数本身只接受`值`类型参数，传递的仍然是值而不是引用。


#### std::bind参数绑定规则
- 占位符：std::placeholders::_n

#### 多线程传参

#### std::mem_fn

#### 关于回调函数
