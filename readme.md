1. Thread pool: 
    1. C style:`g++ -o thread_pool main.cpp -lpthread`
    1. C++ style:`g++ -o thread_pool_cpp thread_pool.cpp -lpthread`
        1. 原理：
            - 使用一个任务队列来存储待处理的任务。queue<TaskType>,using TaskType = std::function<void()>;
                - std::function<返回(参数，参数)>
            - 使用一个线程池来执行任务队列中的任务。vector<thread>
            - 提供一个接口来向任务队列中添加任务。addTask
            - 提供一个接口来停止线程池。stop
            - 使用互斥锁（mutex）和条件变量（condition variable）来实现线程间的同步
            - 构造函数，创建线程池，并启动线程
            - 析构函数，停止线程池，并等待所有线程结束
        2. debug:
            1. 查看子进程pid:`top -H` or `ps -eLf | grep a.out`
2. Memory: `g++ -o memory main.cpp `
    1. 原理：
        - shared_ptr：使用引用计数来管理内存，当引用计数为0时，自动释放内存。
            - 支持拷贝构造函数和赋值操作符，但是需要传递所有权。
            - 支持移动语义，使用std::move()函数将所有权转移给新的shared_ptr。
            - 支持&,*操作符，get()函数，release()释放当前所有权ptr,cnt--。
        - unique_ptr：独占指针，只能有一个指针指向同一块内存，当unique_ptr被销毁时，自动释放内存。
            - 禁用拷贝构造函数和赋值操作符A(const A&) = delete; A& operator=(const A&) = delete;
            - 支持移动语义A(const A&&) = default; A& operator=(const A&&) = default;
            - 支持&,*操作符，get()函数，release()函数释放所有权，返回裸指针。
        - 传递右值：
            ```cpp
            // 1. 显示移动
            munique_ptr<int> ptr1(new int(10));
            munique_ptr<int> ptr2 = std::move(ptr1); // 触发移动构造函数
            munique_ptr<int> ptr3;
            ptr3 = std::move(ptr2); // 触发移动赋值运算符

            // 2. 函数返回值优化
            munique_ptr<int> createPtr() {
                munique_ptr<int> temp(new int(42));
                return temp; // 自动触发移动语义（即使没有显式使用std::move）
            }

            munique_ptr<int> p = createPtr(); // 触发移动构造

            // 3. 标准库容器
            std::vector<munique_ptr<int>> vec;

            // 添加元素（push_back会移动临时对象）
            vec.push_back(munique_ptr<int>(new int(1)));

            // 插入元素（使用std::move）
            munique_ptr<int> p(new int(2));
            vec.push_back(std::move(p));

            // 调整容器大小（元素会被移动）
            vec.resize(10);

            // 4. 函数参数
            void process(munique_ptr<int> ptr) {
                // ...
            }

            munique_ptr<int> original(new int(3));
            process(std::move(original)); // 触发移动构造创建参数

            // 5. 交换对象
            munique_ptr<int> a(new int(10));
            munique_ptr<int> b(new int(20));
            std::swap(a, b); // 内部会使用移动语义进行交换
            ```        
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
            - 使用互斥锁（mutex）来保护临界区，确保同一时间只有一个线程可以访问临界区代码。
            - 使用条件变量（condition variable）来实现线程间的同步，当一个线程需要等待某个条件时，它会释放互斥锁并等待条件变量，当条件满足时，其他线程会通知条件变量，等待的线程会重新获取互斥锁并继续执行。
            - 使用lambda来作为判断条件:`[](){return !this->queue.empty();}`
                ```cpp
                auto condition = []() {
                    return !que.empty();
                };
                cv.wait(lk, condition);
                // ---------------------------------------------------------------------------
                auto add = [](int a, int b) { return a + b; };
                std::cout << add(2, 3); // 输出 5
                // ---------------------------------------------------------------------------
                int x = 10;
                auto f = [x]() { return x + 1; }; // 捕获 x 的副本
                std::cout << f(); // 输出 11
                // ---------------------------------------------------------------------------
                int y = 5;
                auto f2 = [&y]() { y += 10; };
                f2();
                std::cout << y; // 输出 15
                // ---------------------------------------------------------------------------
                ```
5. Memory pool:`g++ -o memory_pool main.cpp`
    1. 原理：
        - 分配内存，blocksize * blocknum,memory=(char *)(malloc(blocksize * blocknum));
        - 处理内存块，void * cur_block = meory; for(int i = 0; i < blocknum; i++) {*(void**)cur_block = cur_block + blocksize; cur_block += blocksize;} freelist = memory;
        - 释放内存，free(memory);
        - allocate():if(freelist == nullptr) {return nullptr;} else {void * ret = freelist; freelist = *(void**)freelist; return ret;}
        - deallocate(ptr): *(void**)ptr = freelist; freelist = ptr; 检查内存是否在范围内。