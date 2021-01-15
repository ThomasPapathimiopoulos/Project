#include "job_scheduler.hpp"

bool running = true;

bool start = false;
std::mutex m_start;
std::condition_variable cv_start;

bool end = false;
std::mutex m_end;
std::condition_variable cv_end;

std::mutex print_mutex;
std::condition_variable cv;
std::mutex m;
std::mutex queue_mutex;
std::mutex train_mutex;

void thread_f(JobScheduler* js){
    while (1){
        //wait for execute_all from job scheduler
        {
            std::unique_lock<std::mutex> lock(m_start);
            cv_start.wait(lock, [] {return start;});
        }

        if(!running){
            break;
        }

        while(js->q->head != NULL){
            queue_mutex.lock();
            bool locked = true;
            if(js->q->head != NULL){
                Job* cj = js->q->pop();
                queue_mutex.unlock();
                locked = false;

                cj->run(); 
            }

            if(locked == true)
                queue_mutex.unlock();
        }

        //wait for wait_for_tasks_to_finish
        {
            std::unique_lock<std::mutex> lock(m_end);
            cv_end.wait(lock, [] {return end;});
        }
    }
}

//============================================================================================================


TestJob::TestJob(unsigned int i){
    id = i;
}

void TestJob::run(){
    print_mutex.lock();
    std::cout << "Job " << id << " executed by ";
    // print_mutex.unlock();
    sleep(1);
}

//============================================================================================================

lr_train_Job::lr_train_Job(std::stringstream* ls, HashTable* hash, SM* f, LR* log_reg) {
    line_stringstream = ls;
    ht = hash;
    files = f;
    lr = log_reg;
}

void lr_train_Job::run() {
    Entry *e1, *e2;
    std::string url1, url2, label_str;
    short int label;
    std::string word, site1, id1, site2, id2;
    size_t first_slash;
    for(int k=0 ; k < 3 ; ++k  ) {
        if(k == 0) {
            getline(*line_stringstream, url1 , ',');
            first_slash = url1.find_first_of('/');
            site1 = url1.substr(0,first_slash);
            id1 = url1.substr(first_slash+2);
        }
        else if(k == 1) {
            getline(*line_stringstream, url2, ',');
            first_slash = url2.find_first_of('/');
            site2 = url2.substr(0,first_slash);
            id2 = url2.substr(first_slash+2);
        }
        else if(k == 2) {
            getline(*line_stringstream, label_str, ',');
            label = atoi(label_str.c_str());       
        }
    }
    e1 = ht->search(hash_value_calculator(site1, id1));
    e2 = ht->search(hash_value_calculator(site2, id2));

    lr->gradient_descent(e1->loc, e2->loc, label, files);
}

//============================================================================================================

JobScheduler::JobScheduler(int n_execution_threads){
    execution_threads = n_execution_threads;
    tids = new std::thread[execution_threads];
    q = new Queue();

    for(int i = 0 ; i < execution_threads ; i++){
        tids[i] = std::thread(thread_f, this);
    }
}

void JobScheduler::execute_all_jobs(){
    start = true;
    end = false;
    cv_start.notify_all();
}

void JobScheduler::wait_all_tasks_finish(){
    start = false;
    end = true;
    cv_end.notify_all();
 
}

void JobScheduler::submit_job(Job* j){
    q->push_back(j);
}

JobScheduler::~JobScheduler(){
    running = false;
    execute_all_jobs();
    for(int i = 0 ; i < execution_threads ; i++) 
        tids[i].join();
    delete q;
    delete[] tids;
}   

