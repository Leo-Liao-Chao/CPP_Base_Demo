1. Thread pool: `g++ -o thread_pool main.cpp -lpthread`
2. Memory: 
3. Thread safe single:`g++ -o thread_safe_single main.cpp`
    1. 原理：
        - 局部静态变量（函数内的 static 变量）只会初始化一次，并且这种初始化是线程安全的。
        - 即：多个线程并发调用 TaskQueue::getInstance() 时，只有一个线程会执行 taskQ 的构造函数，其他线程会等待初始化完成后再返回指针。
4. Multi thread:`g++ -o multi_thread multi_thread.cpp -lpthread`
    1. multi_thread
        1. 原理：
            - 使用互斥锁（mutex）来保护临界区，确保同一时间只有一个线程可以访问临界区代码。
            - 使用条件变量（condition variable）来实现线程间的同步，当一个线程需要等待某个条件时，它会释放互斥锁并等待条件变量，当条件满足时，其他线程会通知条件变量，等待的线程会重新获取互斥锁并继续执行。
        2. debug:
            0. 查看pid:`top` or `ps -ef | grep a.out`
            1. 进入进程:`gdb -p pid`
            2. 查看线程:`info threads`
            3. 查看线程栈:`thread apply all bt` 不太好用
            4. 切换线程:`thread n`
            5. 查看堆栈:`bt`,看哪个进程在wait。
            6. 查看变量:`info locals`
    2. producer_cosumer
        1. 原理：
        