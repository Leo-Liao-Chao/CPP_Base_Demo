#include <pthread.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <list>

class ThreadPool {
 private:
  // 线程池中的工作线程 workers
  struct WORKER {
    pthread_t threadid;      // 线程ID
    bool terminate = false;  // 是否终止线程
    int isWorking = 0;       // 是否正在工作
    ThreadPool *pool;        // 所属线程池
  } *m_workers;

  // 任务结构体
  struct JOB {
    void (*func)(void *arg);  // 任务函数
    void *func_args;          // 任务参数
  };

 public:
  ThreadPool(int numWorkers, int max_jobs)
      : m_max_jobs(max_jobs),
        m_sum_thread(numWorkers),
        m_free_thread(numWorkers) {
    pthread_mutex_init(&m_jobs_mutex, nullptr);  // 初始化互斥锁
    pthread_cond_init(&m_jobs_cond, nullptr);    // 初始化条件变量

    m_workers = new WORKER[numWorkers];  // 初始化工作线程
    for (int i = 0; i < numWorkers; ++i) {
      m_workers[i].pool = this;  // 设置所属线程池
      pthread_create(&m_workers[i].threadid, nullptr, _run,
                     &m_workers[i]);  // 创建线程 （id, 属性, 函数指针, 参数）
    }
  }

  ~ThreadPool() {
    // 通知所有线程退出
    for (int i = 0; i < m_sum_thread; ++i) {
      m_workers[i].terminate = true;
    }

    pthread_cond_broadcast(&m_jobs_cond); // 广播通知所有线程

    for (int i = 0; i < m_sum_thread; ++i) {
      pthread_join(m_workers[i].threadid, nullptr); // 等待线程结束
    }

    pthread_mutex_destroy(&m_jobs_mutex); // 销毁互斥锁
    pthread_cond_destroy(&m_jobs_cond); // 销毁条件变量

    delete[] m_workers; // 删除工作线程数组
  }

  int pushJob(void (*func)(void *data), void *arg, int len) {
    if (func == nullptr) return -1; // 函数指针不能为空

    JOB *job = new JOB; // 创建新的任务
    job->func = func; // 设置任务函数
    job->func_args = malloc(len); // 分配任务参数内存
    memcpy(job->func_args, arg, len); // 复制任务参数

    bool ok = _addJob(job); // 添加任务到队列
    if (!ok) {
      free(job->func_args); // 如果添加失败，释放参数内存
      delete job; // 删除任务对象
      return -1; // 返回失败
    }

    return 0; // 返回成功
  }

 private:
  bool _addJob(JOB *job) {
    pthread_mutex_lock(&m_jobs_mutex);// 锁定互斥锁
    if ((int)m_jobs_list.size() >= m_max_jobs) {
      pthread_mutex_unlock(&m_jobs_mutex); // 如果任务队列已满，解锁并返回失败
      return false;
    }

    m_jobs_list.push_back(job); // 将任务添加到队列
    pthread_cond_signal(&m_jobs_cond); // 通知一个等待的线程有新任务到来
    pthread_mutex_unlock(&m_jobs_mutex);// 解锁互斥锁
    return true;
  }

  static void *_run(void *arg) {
    // pthread_create 的回调函数,arg 是 WORKER 对象的指针
    WORKER *worker = (WORKER *)arg;// 获取工作线程对象
    worker->pool->_threadLoop(arg);// 调用线程循环函数
    return nullptr;
  }

  void _threadLoop(void *arg) {
    WORKER *self = (WORKER *)arg;
    // 线程循环函数
    while (!self->terminate) {
      pthread_mutex_lock(&m_jobs_mutex);// 锁定互斥锁
      while (m_jobs_list.empty() && !self->terminate) { // 如果任务队列为空且线程未终止
        pthread_cond_wait(&m_jobs_cond, &m_jobs_mutex); // 等待条件变量通知, 直到有新任务到来
      }

      if (self->terminate) {// 如果线程被终止，解锁并退出
        pthread_mutex_unlock(&m_jobs_mutex);
        break;
      }

      JOB *job = m_jobs_list.front(); // 获取队列中的第一个任务
      m_jobs_list.pop_front(); // 从队列中移除任务
      self->isWorking = 1; // 标记线程正在工作
      m_free_thread--; // 减少空闲线程计数
      pthread_mutex_unlock(&m_jobs_mutex); // 解锁互斥锁

      job->func(job->func_args);  // 执行任务

      // 函数指针不是动态分配的，是静态分配的，所以不需要释放，在代码段。
      free(job->func_args); // 释放任务参数内存
      delete job; // 删除任务对象

      pthread_mutex_lock(&m_jobs_mutex);// 锁定互斥锁
      self->isWorking = 0; // 标记线程空闲
      m_free_thread++; // 增加空闲线程计数
      pthread_mutex_unlock(&m_jobs_mutex); // 解锁互斥锁
    }
  }

 private:
  std::list<JOB *> m_jobs_list;  // 任务队列
  int m_max_jobs;                // 最大任务数
  int m_sum_thread;              // 总线程数
  int m_free_thread;             // 空闲线程数
  pthread_cond_t m_jobs_cond;    // 任务条件变量
  pthread_mutex_t m_jobs_mutex;  // 任务互斥锁
};

// ==================== 测试 ====================
void myTask(void *arg) {
  int id = *(int *)arg;
  std::cout << "Thread " << pthread_self() << " running task " << id
            << std::endl;
  sleep(1);
}

int main() {
  ThreadPool pool(4, 10);  // 4个线程，最多10个任务

  for (int i = 0; i < 8; ++i) {
    pool.pushJob(myTask, &i, sizeof(int));
    usleep(10000);  // 微小延迟，防止任务参数被覆盖
  }

  sleep(5);  // 等待任务全部完成
  return 0;
}
