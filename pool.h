#ifndef POOL_H_
#define POOL_H_
#include <string>
#include <pthread.h>
#include <deque>
#include <vector>
#include <unordered_map>
#include <utility>

struct thread_data;

class Task {
public:
    Task();
    virtual ~Task();

    virtual void Run() = 0;  // implemented by subclass
};

class ThreadPool {
public:
    std::vector<pthread_t*> thread_array;
    std::deque<std::string> task_queue;
    std::unordered_map<std::string, std::pair<Task*, int>> task_map;
    pthread_mutex_t lock;
    pthread_mutex_t stop_lock;
    pthread_cond_t data_ready;
    std::vector<pthread_mutex_t> task_locks;
    std::vector<pthread_cond_t> conds;
    std::vector<int> thread_ids;
    int thread_num_pool;
    bool stop;
    static void* worker(void* arg);

    
    ThreadPool(int num_threads);
    ~ThreadPool();
    std::vector<thread_data*> datum;
    // void* thread_compute(void* data_arg);
    // Submit a task with a particular name.
    void SubmitTask(const std::string &name, Task *task);
 
    // Wait for a task by name, if it hasn't been waited for yet. Only returns after the task is completed.
    void WaitForTask(const std::string &name);

    // Stop all threads. All tasks must have been waited for before calling this.
    // You may assume that SubmitTask() is not caled after this is called.
    void Stop();

};

struct thread_data{
    int thread_id;
    ThreadPool* pool;
};

#endif
