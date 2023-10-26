#ifndef POOL_H_
#include <string>
#include <pthread.h>
#include <deque>
#include <vector>
#include <unordered_map>
#include <utility>

class Task {
public:
    Task();
    virtual ~Task();

    virtual void Run() = 0;  // implemented by subclass
};

class ThreadPool {
private:
    vector<pthread_t*> thread_array;
    deque<string> task_queue;
    unordered_map<std::string, pair<Task*, int> > task_map;
    pthread_mutex_t lock;
    vector<pthread_mutex_t> task_locks;
    vector<pthread_cond_t> conds;
    vector<int> thread_ids;
    pthread_cond_t data_ready;
    bool stop;
    static void* worker(void* arg);

public:
    ThreadPool(int num_threads);
    void* thread_compute();
    // Submit a task with a particular name.
    void SubmitTask(const std::string &name, Task *task);
 
    // Wait for a task by name, if it hasn't been waited for yet. Only returns after the task is completed.
    void WaitForTask(const std::string &name);

    // Stop all threads. All tasks must have been waited for before calling this.
    // You may assume that SubmitTask() is not caled after this is called.
    void Stop();

};

#endif
