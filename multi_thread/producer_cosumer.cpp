#include <algorithm>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
using namespace std;

int N = 100;

const int MAX_SIZE = 10;

std::condition_variable cv;
std::mutex mtx;
std::queue<int> que;

void producer() {
    for (int i = 0; i < N; ++i) {
        std::unique_lock<mutex> lk(mtx);
        cv.wait(lk, []() {
            return que.size() < MAX_SIZE;
        }); // lambda表达式，返回true表示继续执行，返回false表示阻塞等待
        std::cout << std::this_thread::get_id() << " 生成数据 " << i <<" 使用容量 "<<que.size()<< endl;
        que.push(i);
        cv.notify_one();
    }
}

void consumer() {
    for (int i = 0; i < N; ++i) {
        std::unique_lock<mutex> lk(mtx);
        cv.wait(lk, []() {
            return !que.empty(); // 当前线程拿着锁 mtx，并且等待队列 que 非空，一旦非空就继续执行。
        });
        std::cout << std::this_thread::get_id() << " 消耗数据 " << que.front()<<" 使用容量 "<<que.size() << endl;
        que.pop();
        cv.notify_one();
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();
}