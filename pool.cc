#include "pool.h"

#define pb push_back
//helper function to grab tasks from queue

Task::Task() {

}
Task::~Task() {
}  
void* thread_compute(void* data_arg){
    
    thread_data* thread_data_inst = static_cast<thread_data*>(data_arg);
    std::deque<std::string> *task_queue = &thread_data_inst->pool->task_queue;
    std::unordered_map<std::string, std::pair<Task*, int>> *task_map = &thread_data_inst->pool->task_map;
    pthread_mutex_t *lock = &thread_data_inst->pool->lock;
    pthread_mutex_t *stop_lock = &thread_data_inst->pool->stop_lock;
    pthread_cond_t *data_ready = &thread_data_inst->pool->data_ready;
    std::vector<pthread_mutex_t> *task_locks = &thread_data_inst->pool->task_locks;;
    std::vector<pthread_cond_t> *conds = &thread_data_inst->pool->conds;
    bool *stop = &thread_data_inst->pool->stop;
    int* thread_id = &thread_data_inst->thread_id;
    bool is_stop = false;
    while(!is_stop){
        pthread_mutex_lock(stop_lock);
            is_stop = *stop;
            //assign thread id to task
            while (it != task_map->end()) {
                if(it->first==item){
                    it->second.second=*thread_id;
                    break;
                }
                it++;
            }
        pthread_mutex_unlock(stop_lock);
        pthread_mutex_lock(lock);
        while ((*task_queue).empty()){
            pthread_cond_wait(data_ready, lock);
        }
        std::string item = (*task_queue).front();
        (*task_queue).pop_front();
        auto it = task_map->begin();
        Task* item_function = (*task_map)[item].first;
        if (item_function != nullptr){
            item_function->Run();
        }
        int temp_int = *thread_id;
        it->second.second = -2;
        pthread_mutex_unlock(lock);
        // pthread_mutex_unlock(&(*task_locks)[temp_int]);
        pthread_cond_signal(&(*conds)[temp_int]);
    }
    

    return nullptr;
}


ThreadPool::ThreadPool(int num_threads) {
    thread_num_pool = num_threads;
    task_locks.resize(num_threads);
    conds.resize(num_threads);
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&stop_lock, NULL);
    pthread_cond_init(&data_ready, NULL);
    for (int i = 0; i < num_threads; i++){
        pthread_mutex_init(&task_locks[i], NULL);
        thread_ids.push_back(i);
        pthread_cond_init(&conds[i], NULL);
    } 

    stop = false;
    datum.resize(num_threads);  
    for(int i = 0; i < num_threads; i++){
        //create new threads
        thread_array.pb(new pthread_t);      
        //start threads with helper function
        datum[i] = new thread_data;
        datum[i]->thread_id = i;
        datum[i]->pool = this;
        pthread_create(thread_array[i], NULL, thread_compute, datum[i]);
    }


}
ThreadPool::~ThreadPool(){
    for(int i = 0; i < thread_num_pool; i++){
        delete thread_array[i];
        pthread_mutex_destroy(&task_locks[i]);
        pthread_cond_destroy(&conds[i]);
        delete datum[i];
    }
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&data_ready);
}
void ThreadPool::SubmitTask(const std::string &name, Task* task) {

    pthread_mutex_lock(&lock);
    task_map[name] = std::make_pair(task, -3);
    task_queue.pb(name);
    pthread_cond_signal(&data_ready);
    pthread_mutex_unlock(&lock);
}

void ThreadPool::WaitForTask(const std::string &name) {
    pthread_mutex_lock(stop_lock);
     int thread_index = task_map[name].second;
     if(thread_index<0){
        return;
     }
     pthread_mutex_t* task_lock = &datum[thread_index]->pool->task_locks[thread_index];
     pthread_mutex_lock(task_lock);
     while(task_map[name].second<=-1){
        pthread_cond_wait(&conds[thread_index],&task_locks[thread_index]);
     }
     pthread_mutex_unlock(task_lock);
 }

void ThreadPool::Stop() {
    pthread_mutex_lock(&lock);
    for(int i = 0; i < thread_num_pool; i++){
        pthread_join(*thread_array[i],NULL);
    }
    pthread_mutex_unlock(&lock);
    delete this;
}
