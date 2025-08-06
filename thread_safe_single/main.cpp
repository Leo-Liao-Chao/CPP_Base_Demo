#include <iostream>
using namespace std;

class TaskQueue {
public:
    TaskQueue& operator=(const TaskQueue& obj) = delete;// 禁止拷贝赋值
    static TaskQueue* getInstance() {
        static TaskQueue taskQ;
        return &taskQ;
    }
    void print() {
        cout << "TaskQueue instance address: " << this << endl;
    }
private:
    // 私有构造函数，禁止外部创建实例
    TaskQueue() {}

};

int main() {
    TaskQueue* queue = TaskQueue::getInstance();
    queue->print();
    return 0;
}