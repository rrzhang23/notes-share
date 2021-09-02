# [C++并发实战](./Cpp_Concurrency_In_Action.pdf.pdf)


- [] 线程传参：

~~~cpp
class X
{
public:
void do_lengthy_work(int);
};
X my_x;
int num(0);
std::thread t(&X::do_lengthy_work, &my_x, num);
~~~

- [] 获取硬件线程数量：std::thread::hardware_concurrency()
- [] std::thread::id 实例常用作检测线程是否需要进行一些操作
- [] 书中list2.6用析构函数保证线程正确join，lock_guard保证正确unlock()。
## 第三章 共享数据并发控制
这一章可参考https://www.jianshu.com/p/835e80035ea8
对于数据结构（stack，queue等）当中函数加锁的情况，仍然存在竞争问题，应当对整
个结构体加锁。
避免死锁：多个互斥量上锁 std::lock(),以及std::lock() 和 std::lock_guard的区别
std::once_flag和std::call_once来保证共享数据初始化的正确性。
Cpp_Concurrency… n.pdf
4.64MBboost::shared_mutex 可以共享锁可以保证多个线程同时读。
look_guard
unique_guard(参考https://www.cnblogs.com/xudong-bupt/p/9194394.html）
std::defer_lock等
## 第四章 同步
std::chrono使用
1. std::condition_variable_any配合wait()、notify_one()实现同步，仍然使用
std::mutex。（wait()函数可以使用lamda作为参数 ）2.std::future、std::promise、std::packaged_task 、std::async（参考
https://www.cnblogs.com/jiayayao/p/6527989.html，这篇文章简单梳理了thread，
packaged_task,async之间的关系，以及std::launch的两个策略来控制何时启动一个线
程）。
## 第5章 C++内存模型和原子类型操作
参考http://www.cnblogs.com/taiyang-li/p/5914331.html
std::atomic使用，除了库结构，可重载使用。
使用atomic_flag可实现mutex.
利用std::atomic可实现数据结构的无锁设计。ATOMIC_FLAG_INIT进行初始化。
另一种使用场景，替代std::condition_variable实现同步。
（https://blog.csdn.net/what951006/article/details/78273903）
#### 5.2.4 std::atomic:指针运算
5.3.3线程间原子操作的内存顺序，参考：
https://www.zhihu.com/question/24301047
https://blog.csdn.net/what951006/article/details/78273903
#### 5.3.5栅栏
std::atomic_thread_fence


第二版：
github翻译：
https://github.com/xiaoweiChen/CPP-Concurrency-In-Action-2ed-2019

原书：
https://livebook.manning.com/#!/book/c-plus-plus-concurrency-in-action-secondedition/chapter-1/v-7