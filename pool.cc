#include "pool.h"

#define pb push_back


Task::Task() {

}
Task::~Task() {
  
}

//helper function to grab tasks from queue
void* thread_compute(int* thread_id){
    task_map[name].second = thread_id; 
    while(!stop){
        pthread_mutex_lock(&lock);
        while (task_queue.empty()){
            pthread_cond_wait(&data_ready, &lock);
        }
        std::string item = task_queue.pop_front();
        pthread_mutex_unlock(&lock);
        void* item_function = task_map[item].first;
        pthread_mutex_lock(&task_locks[thread_id*]);
        item_function*();
        pthread_mutex_unlock(&task_locks[thread_id*]);
        pthread_cond_signal(&conds[thread_id*]);
    }


}

ThreadPool::ThreadPool(int num_threads) {
    task_locks.resize(num_threads);
    conds.resize(num_threads);
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&data_ready, NULL);
    for (int i = 0; i < num_threads; i++){
        pthread_mutex_init(&task_locks[i], NULL);
        thread_ids.push_back(i);
        pthread_cond_init(&conds[i], NULL);
    } 

    stop = false;
    for(int i = 0; i < num_threads; i++){
        //create new threads
        thread_array.pb(new pthread_t);      

        //start threads with helper function
        pthread_create(thread_array[i], NULL, thread_compute, &thread_ids[i]);
    }


}
ThreadPool::~ThreadPool(){
    for(int i = 0; i < num_threads; i++){
        delete thread_array[i];
    }
    for(int i = 0; i < num_threads; i++){
        pthread_mutex_destroy(&task_locks[i]);
    }
    for(int i = 0; i < num_threads; i++){
        pthread_mutex_destroy(&conds[i]);
    }
    // delete[] thread_array;
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&data_ready);
}
void ThreadPool::SubmitTask(const std::string &name, Task* task) {
    pthread_mutex_lock(&lock);
    task_map[name] = make_pair(task, -1);
    task_queue.pb(name);
    pthread_cond_signal(&data_ready);
    pthread_mutex_unlock(&lock);
}

void ThreadPool::WaitForTask(const std::string &name) {
     Task* active_task = task_map[name];
     int thread_index = task_map[name].second;
     pthread_cond_wait(&conds[thread_index],&task_locks[thread_index]);
 }

void ThreadPool::Stop() {
    pthread_mutex_lock(&lock);
    for(int i = 0; i < threads; i++){
        pthread_join(*thread_array[i],NULL);
    }
    pthread_mutex_unlock(&lock);
}
