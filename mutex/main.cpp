#include <atomic>
#include <thread>
#include <iostream>

class Mutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        // 自旋等待，直到成功将 flag 设为 true（锁住）
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 等待期间让出CPU，避免空转太浪费
            std::this_thread::yield();
        }
    }

    void unlock() {
        // 释放锁，将 flag 清零
        flag.clear(std::memory_order_release);
    }
};

// RAII 机制的锁保护类
class LockGuard {
    Mutex& mtx;
public:
    // explicit 是为了防止隐式转换
    explicit LockGuard(Mutex& m) : mtx(m) {
        mtx.lock();
    }
    ~LockGuard() {
        mtx.unlock();
    }
    // 禁止拷贝和赋值
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

// 测试
int main() {
    Mutex mtx;
    int counter = 0;

    auto worker = [&]() {
        for (int i = 0; i < 100000; ++i) {
            LockGuard lock(mtx);
            ++counter;
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();

    std::cout << "counter = " << counter << std::endl;
    return 0;
}
