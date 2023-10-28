#ifndef POOL_H_
#define POOL_H_

#include <string>
#include <pthread.h>
#include <vector>
#include <deque>
#include <unordered_map>
#include <atomic>


class Task {
public:
    std::atomic<bool> completed;
    pthread_mutex_t task_lock;
    pthread_cond_t task_ready;
    Task();
    virtual ~Task();

    virtual void Run() = 0;  // implemented by subclass
};

class ThreadPool {
public:
    int num_threads_pool; //number of threads in the pool
    std::unordered_map<std::string, Task*> task_map; //map string name to function
    pthread_mutex_t queue_lock; //protect the task deque
    pthread_cond_t data_ready; //to identify if data on deque
    std::vector<pthread_t*> thread_list; //vector of threads needed to delete threads
    pthread_mutex_t stop_lock; //protect stop bool
    std::atomic<bool> is_stop;
    std::deque<std::string> task_queue;
    //std::vector<Task*> delete_task_list;

    ThreadPool(int num_threads);

    // Submit a task with a particular name.
    void SubmitTask(const std::string &name, Task *task);
 
    // Wait for a task by name, if it hasn't been waited for yet. Only returns after the task is completed.
    void WaitForTask(const std::string &name);

    // Stop all threads. All tasks must have been waited for before calling this.
    // You may assume that SubmitTask() is not caled after this is called.
    void Stop();

    ~ThreadPool(); //destructor
};

    //worker thread within pool that completes tasks
    void* thread_compute(void* data_arg);
#endif
