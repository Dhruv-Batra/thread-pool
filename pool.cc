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
    std::unordered_map<std::string, struct TaskInfo> *task_map = &thread_data_inst->pool->task_map;
    pthread_mutex_t *lock = &thread_data_inst->pool->lock;
    pthread_mutex_t *stop_lock = &thread_data_inst->pool->stop_lock;
    pthread_mutex_t *task_map_lock = &thread_data_inst->pool->task_map_lock;
    pthread_cond_t *data_ready = &thread_data_inst->pool->data_ready;
    //std::vector<pthread_mutex_t> *task_locks = &thread_data_inst->pool->task_locks;
    // std::vector<pthread_mutex_t> *task_assignments = &thread_data_inst->pool->task_assignments;
    bool *stop = &thread_data_inst->pool->stop;
    int* thread_id = &thread_data_inst->thread_id;
    bool is_stop = false;
    while(!is_stop){
        //pthread_mutex_lock(&(*task_assignments)[*thread_id]);
        pthread_mutex_lock(stop_lock);
        is_stop = *stop;
        pthread_mutex_unlock(stop_lock);
        pthread_mutex_lock(lock);
        while ((*task_queue).empty()){
            pthread_cond_wait(data_ready, lock);
        }
        std::string item = (*task_queue).front();
        (*task_queue).pop_front();
        pthread_mutex_unlock(lock);
        //assign thread id to task
        pthread_mutex_lock(task_map_lock);
        auto it = task_map->begin();
        while (it != task_map->end()) {
            if(it->first==item){
                it->second.t_id=*thread_id;
                break;
            }
            it++;
        }
        Task* item_function = (*task_map)[item].task;
        pthread_mutex_unlock(task_map_lock);
        
        pthread_mutex_lock((*task_map)[item].mutech);
        //pthread_mutex_lock(&(*task_locks)[*thread_id]);
        if (item_function != nullptr){
            item_function->Run();
        }
        //int temp_int = *thread_id;
        //pthread_mutex_unlock(&(*task_locks)[temp_int]);
        pthread_mutex_lock(task_map_lock);
        it->second.t_id = -2;
        pthread_mutex_unlock(task_map_lock);
        pthread_mutex_unlock((*task_map)[item].mutech);
        pthread_cond_signal((*task_map)[item].completed_task);

        //pthread_mutex_unlock(&(*task_assignments)[*thread_id]);
        //pthread_cond_signal(&(*conds_assignments)[*thread_id]);
        //pthread_cond_signal(&(*conds)[temp_int]);
    }
    

    return nullptr;
}


ThreadPool::ThreadPool(int num_threads) {
    thread_num_pool = num_threads;
    task_locks.resize(num_threads);
    //task_assignments.resize(num_threads);
    conds.resize(num_threads);
    //conds_assignments.resize(num_threads);
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&task_map_lock, NULL);
    pthread_mutex_init(&stop_lock, NULL);
    pthread_cond_init(&data_ready, NULL);
    // for (int i = 0; i < num_threads; i++){
    //     //pthread_mutex_init(task_locks[i], NULL);
    //     //pthread_mutex_init(&task_assignments[i], NULL);
    //     thread_ids.push_back(i);
    //     pthread_cond_init(conds[i], NULL);
    //     //pthread_cond_init(&conds_assignments[i], NULL);
    // } 

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
        pthread_mutex_destroy(task_locks[i]);
        pthread_cond_destroy(conds[i]);
        delete datum[i];
    }
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&data_ready);
    for (auto const &pair:task_map){

        pthread_cond_destroy(pair.second.completed_task);
        delete pair.second.completed_task;
        pthread_mutex_destroy(pair.second.mutech);
        delete pair.second.mutech;
    }
}
void ThreadPool::SubmitTask(const std::string &name, Task* task) {
    pthread_mutex_t* temp_mutex = new pthread_mutex_t;
    pthread_mutex_init(temp_mutex, NULL);
    task_locks.push_back(temp_mutex);
    pthread_cond_t* temp_cond = new pthread_cond_t;
    pthread_cond_init(temp_cond, NULL);
    conds.push_back(temp_cond);
    pthread_mutex_lock(&task_map_lock);
    TaskInfo info = {task, -3,temp_cond, temp_mutex};
    task_map[name] = info;
    pthread_mutex_unlock(&task_map_lock);
    pthread_mutex_lock(&lock);
    task_queue.pb(name);
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&data_ready);
    
}

void ThreadPool::WaitForTask(const std::string &name) {
    
    pthread_mutex_lock(&task_map_lock);
    int temp_index=task_map[name].t_id; 
     pthread_mutex_unlock(&task_map_lock);
     pthread_mutex_lock(task_map[name].mutech);
     while(temp_index!=-2){//while thread int
        pthread_cond_wait(task_map[name].completed_task, task_map[name].mutech);
        pthread_mutex_lock(&task_map_lock);
        temp_index = task_map[name].t_id;
        pthread_mutex_unlock(&task_map_lock);
        //int wait_me = task_map[name].second;
     }
     pthread_mutex_unlock(task_map[name].mutech);
    //  pthread_mutex_lock(&task_map_lock);
    //  temp_index = task_map[name].second;
    //  pthread_mutex_unlock(&task_map_lock);
    //  if(temp_index==-2){
    //     return;
    //  }
    // //pthread_mutex_lock(&task_locks[temp_index]);
    //  pthread_mutex_lock(&task_map_lock);
    //  temp_index=task_map[name].second; 
    //  pthread_mutex_unlock(&task_map_lock);
    //  while(temp_index ==-2){
    //     pthread_mutex_lock(&task_map_lock);
    //     temp_index=task_map[name].second; 
    //     pthread_mutex_unlock(&task_map_lock);
    //  }
    //  //pthread_mutex_t* task_lock = &datum[thread_index]->pool->task_locks[thread_index];
    // //  pthread_mutex_unlock(&task_locks[temp_index]);
    // pthread_mutex_lock(&task_map_lock);
    // task_map[name].second = -2;
    // pthread_mutex_unlock(&task_map_lock);
 }

void ThreadPool::Stop() {
    pthread_mutex_lock(&lock);
    for(int i = 0; i < thread_num_pool; i++){
        pthread_join(*thread_array[i],NULL);
    }
    pthread_mutex_unlock(&lock);
    delete this;
}
