1. Thread pool: `g++ -o thread_pool main.cpp -lpthread`
2. Memory: 
3. Thread safe single:`g++ -o thread_safe_single main.cpp`
    1. 原理：
        - 局部静态变量（函数内的 static 变量）只会初始化一次，并且这种初始化是线程安全的。
        - 即：多个线程并发调用 TaskQueue::getInstance() 时，只有一个线程会执行 taskQ 的构造函数，其他线程会等待初始化完成后再返回指针。
4. 