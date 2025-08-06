#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
using namespace std;

class ThreadPool {
    using TaskType = std::function<void()>;
public:
    ThreadPool(int n) : nthread(n) {
        works.reserve(n);
        for (int i = 0; i < n; ++i) {
            works.push_back(thread([this]() {
                while (true) {
                    std::unique_lock<mutex> lk(mtx);
                    cv.wait(lk, [this]() {
                        return !tasks.empty() || stop;
                    });
                    if (tasks.empty() && stop) {
                        return;
                    }
                    TaskType task = std::move(tasks.front());
                    tasks.pop();
                    task();
                }
            }));
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<mutex> lk(mtx);
            stop = true;
        }
        cv.notify_all();
        for (int i = 0; i < nthread; ++i) {
            works[i].join();
        }
    }

    void setStop(bool flag = true) {
        {
            std::unique_lock<mutex> lk(mtx);
            stop = true;
        }
        cv.notify_all();
    }

    void addTask(TaskType task) {
        {
            std::unique_lock<mutex> lk(mtx);
            tasks.push(task);
        }
        cv.notify_one();
    }

private:
    int nthread;
    bool stop = false; // 默认是没退出
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<thread> works;
    std::queue<TaskType> tasks;
};

int main() {
    ThreadPool pool(10);
    for (int i = 0; i < 1000000000; ++i) {
        pool.addTask([x = i]() { // 注意这里变量的捕获需要用值捕获
            cout << std::this_thread::get_id() << " 正在处理任务 " << x << endl;
        });
    }
    return 0;
}