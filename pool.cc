#include "pool.h"

Task::Task() {
    completed=false;
    pthread_mutex_init(&(this->task_lock), NULL);
    pthread_cond_init(&(this->task_ready), NULL);
}

Task::~Task() {
    pthread_mutex_destroy(&(this->task_lock));
    pthread_cond_destroy(&(this->task_ready));
}

void* thread_compute(void* data_arg){
    ThreadPool* tp_inst = static_cast<ThreadPool*>(data_arg);
    while(true){
        if(tp_inst->is_stop==true){
            break;
        }
        pthread_mutex_lock(&(tp_inst->queue_lock));
        while (true){
            if(!((tp_inst->task_queue).empty()) || tp_inst->is_stop){
                break;
            }
            pthread_cond_wait(&(tp_inst->data_ready), &(tp_inst->queue_lock));
        }
        if (tp_inst->task_queue.empty()) {
            pthread_mutex_unlock(&(tp_inst->queue_lock));
            break;
        }
        std::string item = (tp_inst->task_queue).front();
        (tp_inst->task_queue).pop_front();
        Task* active_task = tp_inst->task_map[item];
        pthread_mutex_unlock(&(tp_inst->queue_lock));

        pthread_mutex_lock(&(active_task->task_lock));
        active_task->Run();
        active_task->completed=true;
        pthread_mutex_unlock(&(active_task->task_lock));

        pthread_cond_signal(&(active_task->task_ready));

    }
    return nullptr;
}

ThreadPool::ThreadPool(int num_threads) {
    num_threads_pool = num_threads;
    is_stop=false;
    pthread_mutex_init(&queue_lock, NULL);
    pthread_cond_init(&data_ready, NULL);
    //iterate though - create threads then add to vector
    for(int i=0; i<num_threads; i++){
        pthread_t* temp_thread = new pthread_t;
        thread_list.push_back(temp_thread);
        pthread_create(thread_list[i], NULL, thread_compute, this);
    }
}

void ThreadPool::SubmitTask(const std::string &name, Task* task) {
    pthread_mutex_lock(&queue_lock);
    task_map[name] = task;
    task_queue.push_back(name);
    pthread_mutex_unlock(&queue_lock);
    pthread_cond_signal(&data_ready);
}

void ThreadPool::WaitForTask(const std::string &name) {
    pthread_mutex_lock(&(task_map[name]->task_lock));
    while(!task_map[name]->completed){
        pthread_cond_wait(&(task_map[name]->task_ready), &(task_map[name]->task_lock));
    }
    pthread_mutex_unlock(&(task_map[name]->task_lock));
    
    pthread_mutex_lock(&queue_lock);
    delete task_map[name];
    task_map.erase(name);
    pthread_mutex_unlock(&queue_lock);
}

void ThreadPool::Stop() {
    is_stop=true;
    pthread_cond_broadcast(&data_ready);
    for(int i = 0; i < num_threads_pool; i++){
        pthread_join(*thread_list[i],NULL);
    }
}

ThreadPool::~ThreadPool(){
    for(int i=0; i<num_threads_pool; i++){
        delete thread_list[i];
    }
    pthread_mutex_destroy(&queue_lock);
    pthread_cond_destroy(&data_ready);
    // for (auto& pair : task_map) {
    //     delete pair.second;
    // }
    // task_map.clear();
}
