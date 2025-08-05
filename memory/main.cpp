#include <iostream>
using namespace std;

template<typename T>
class mshared_ptr {
private:
    T* _ptr;
    int* _cnt;
public:
    mshared_ptr() : _ptr(nullptr), _cnt(new int(0)) {
        cout << "shared_ptr: 默认构造\n";
    }
    // explicit 是为了防止隐式转换
    explicit mshared_ptr(T* ptr) : _ptr(ptr), _cnt(new int(1)) {
        cout << "shared_ptr: 构造 ptr\n";
    }

    // 拷贝构造
    mshared_ptr(const mshared_ptr& other) : _ptr(other._ptr), _cnt(other._cnt) {
        if (_cnt) ++(*_cnt); // 引用计数加1
        cout << "shared_ptr: 拷贝构造\n";
    }

    // 移动构造
    mshared_ptr(mshared_ptr&& other) noexcept : _ptr(other._ptr), _cnt(other._cnt) {
        other._ptr = nullptr; // 将原对象的指针置空
        other._cnt = nullptr; // 将原对象的引用计数置空
        cout << "shared_ptr: 移动构造\n";
    }

    // 拷贝赋值
    mshared_ptr& operator=(const mshared_ptr& other) {
        if (this != &other) {
            release(); // 释放当前对象的资源
            _ptr = other._ptr; // 指向其他对象的指针
            _cnt = other._cnt; // 引用计数
            if (_cnt) ++(*_cnt);  // 引用计数加1
        }
        return *this;
    }

    // 移动赋值
    mshared_ptr& operator=(mshared_ptr&& other) noexcept {
        if (this != &other) {
            release();
            _ptr = other._ptr;// 将指针指向其他对象的指针
            _cnt = other._cnt;// 将引用计数指向其他对象的引用计数
            other._ptr = nullptr; // 将其他对象的指针置空
            other._cnt = nullptr;// 将其他对象的引用计数置空
        }
        return *this;
    }

    ~mshared_ptr() {
        release();// 释放资源
    }

    void release() {
        if (_cnt && --(*_cnt) == 0) { // 引用计数减1，如果为0，释放资源
            delete _ptr; // 删除指针指向的资源
            delete _cnt; // 删除引用计数
            cout << "shared_ptr: 析构删除资源\n";
        }
    }

    T& operator*() const { return *_ptr; } // 解引用
    T* operator->() const { return _ptr; } // 成员访问
    int count() const { return _cnt ? *_cnt : 0; } // 返回引用计数
    T* get() const { return _ptr; } // 返回指针
};

template<typename T>
class munique_ptr {
private:
    T* _ptr;
public:
    munique_ptr() : _ptr(nullptr) {
        cout << "unique_ptr: 默认构造\n";
    }
    // explicit 是为了防止隐式转换
    explicit munique_ptr(T* ptr) : _ptr(ptr) {
        cout << "unique_ptr: 构造 ptr\n";
    }

    // 禁用拷贝构造和赋值
    munique_ptr(const munique_ptr&) = delete;
    munique_ptr& operator=(const munique_ptr&) = delete;

    // 支持移动
    munique_ptr(munique_ptr&& other) noexcept : _ptr(other._ptr) {
        other._ptr = nullptr;
        cout << "unique_ptr: 移动构造\n";
    }
    // 移动
    munique_ptr& operator=(munique_ptr&& other) noexcept {
        if (this != &other) {
            delete _ptr;// 删除当前对象的指针
            _ptr = other._ptr; // 将指针指向其他对象的指针
            other._ptr = nullptr; // 将其他对象的指针置空
        }
        return *this;
    }

    ~munique_ptr() {
        if (_ptr) {
            delete _ptr; // 删除指针指向的资源
            cout << "unique_ptr: 析构删除资源\n";
        }
    }

    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }
    T* get() const { return _ptr; }

    // 释放所有权
    T* release() {
        T* tmp = _ptr; // 保存指针
        _ptr = nullptr; // 将指针置空
        return tmp; // 返回保存的指针
    }
    // 重新设置指针
    void reset(T* ptr = nullptr) {
        if (_ptr) delete _ptr;
        _ptr = ptr;
    }
};

template<typename T>
typename std::remove_reference<T>::type&& my_move(T&& arg) {
    return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

struct Person {
    string name;
    Person(string n) : name(n) {
        cout << name << " 构造\n";
    }
    ~Person() {
        cout << name << " 析构\n";
    }
    void sayHi() const {
        cout << name << " says Hi!\n";
    }
};

int main() {
    {
        cout << "\n===== unique_ptr 测试 =====\n";
        munique_ptr<Person> u1(new Person("Alice"));
        u1->sayHi();

        munique_ptr<Person> u2 = my_move(u1);  // 移动构造
        if (!u1.get()) cout << "u1 已被转移\n";
        u2->sayHi();
    }

    {
        cout << "\n===== shared_ptr 构造+引用计数 测试 =====\n";
        mshared_ptr<Person> s1(new Person("Bob"));
        {
            mshared_ptr<Person> s2 = s1; // 拷贝构造
            cout << "引用计数: " << s1.count() << endl;
            s2->sayHi();
        }
        cout << "引用计数: " << s1.count() << endl;
    }

    {
        cout << "\n===== shared_ptr 拷贝赋值 测试 =====\n";
        mshared_ptr<Person> p1(new Person("Cindy"));
        mshared_ptr<Person> p2;
        p2 = p1; // 拷贝赋值
        cout << "引用计数: " << p1.count() << endl;
    }

    {
        cout << "\n===== shared_ptr 移动赋值 测试 =====\n";
        mshared_ptr<Person> p1(new Person("David"));
        mshared_ptr<Person> p3;
        p3 = my_move(p1); // 移动赋值
        if (!p1.get()) cout << "p1 已被移动\n";
        cout << "引用计数: " << p3.count() << endl;
        p3->sayHi();
    }

    cout << "\n===== 程序结束 =====\n";
    return 0;
}
