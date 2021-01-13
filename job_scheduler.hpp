#ifndef _JOB_SCHEDULER_
#define _JOB_SCHEDULER_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <thread>
#include <mutex>
#include <pthread.h>
#include <dirent.h>
#include "entry.hpp"
#include "hashtable.hpp"
#include "parser.hpp"
#include "dict.hpp"
#include "logistic_regression.hpp"
#include "sparse_matrix.hpp"

class Job{
public:
    virtual void run() = 0;
    virtual ~Job() { }
};

class vectorify_Job : public Job{
private:
    Dict *all_words;
    std::string *all_words_vector;
    float *all_idf_vector;
    float *all_tfidf_sum_vector;

    unsigned int *loc;
    int num_entries;
public:
    vectorify_Job(Dict* n_all_words, std::string * n_all_words_vector, float * n_all_idf_vector, float * n_all_tfidf_sum_vector, unsigned int * n_loc, int n_num_entries){
        all_words = n_all_words;
        all_words_vector = n_all_words_vector;
        all_idf_vector = n_all_idf_vector;
        all_tfidf_sum_vector = n_all_idf_vector;
        loc = n_loc;
        num_entries = n_num_entries;
    };
    void run() override {
        all_words->vectorify(all_words->root, all_words_vector, all_idf_vector, all_tfidf_sum_vector, loc, num_entries);
    }
};

class create_database_Job : public Job {
    Dict* all_words;
    Parser* p;
    dirent *folder;
    HashTable* ht;
    Clique* list_of_entries;
    pthread_mutex_t* mutex;

public:
    create_database_Job(Dict* aw, Parser* parser, dirent* dir, HashTable* hash, Clique* loe, pthread_mutex_t* m) {
        all_words = aw;
        p = parser;
        folder = dir;
        ht = hash;
        list_of_entries = loe;
        mutex = m;
    }
    void run() override {
        // std::cout << "folder = " << folder->d_name << std::endl;
        // for each json use name for constructor
        std::string file_dir = "./Datasets/2013_camera_specs/";
        file_dir.append(folder->d_name); //create every folders path
        // std::cout << file_dir << "\n";
        struct dirent *file;
        Entry* e;
        DIR *dir_f;
        if ( (dir_f = opendir(file_dir.c_str())) == NULL ) { //open the created folder path to access jsons
            perror("can't open the given directory");
            exit(2);
        } else {
            while ( (file = readdir(dir_f)) ) {
                if (file->d_name != std::string(".") && file->d_name != std::string("..")) {
                    // std::cout << "folder = " << folder->d_name << "file = " << file->d_name << std::endl;
                    std::string id_str = file->d_name; //keep id of product from file's title
                    size_t lastdot = id_str.find_last_of(".");

                    std::string id = id_str.substr(0, lastdot);
                    // std::cout << "folder = " << folder->d_name << " file = " << id_str << std::endl;
                    // call entry constructor and insert to entry_list and hashtable
                    std::string path = file_dir+"/"+id_str;

                    pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
                    e = new Entry(std::string(folder->d_name), id, p->parse(path));
                    e->specs_words = e->get_specs()->clean_up();
                    // std::cout << counter << std::endl;
                    // counter++;
                    all_words->root = all_words->add(all_words->root, e->specs_words, &(e->specs_words));
                    // e->get_specs()->print();
                    list_of_entries->push(e);
                    ht->insert(e);
                    pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
                }
            }

            (void) closedir (dir_f);
        }
    }
};

class print_clique_Job : public Job {
    std::ofstream* output;
    unsigned int* output_lines_counter;
    Clique* clique;
    pthread_mutex_t* mutex;

public:
    print_clique_Job(Clique* c, std::ofstream* out, unsigned int* lc, pthread_mutex_t* m) {
        clique = c;
        output = out;
        output_lines_counter = lc;
        mutex = m;
    }
    void run() {
        int size = clique->size; 
        Cliquenode* table[size];            //create table of clique's members
        Cliquenode* temp_entry = clique->head;
        for(int i=0 ; i < size ; i++) {
            table[i] = temp_entry;
            temp_entry = temp_entry->next;
        }

        // print every possible pair from that clique
        for(int i=0 ; i<size ; i++) {       
            std::string url1 = table[i]->data->get_page_title() + "//" + table[i]->data->get_id();
            for(int j=i+1 ; j<size ; j++) {
                std::string url2 = table[j]->data->get_page_title() + "//" + table[j]->data->get_id();
                pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
                *output << url1 << "," << url2 << ",1" <<  "\n";
                *output_lines_counter += 1;
                pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
            }
            // table[i]->data->clique = NULL;  //make clique pointer NULL for all those entries so we don't print any pair more than once
        }
    }
};

class print_anticlique_Job : public Job {
    std::ofstream* output;
    unsigned int* output_lines_counter;
    Clique* clique;
    Clique* anticlique;
    pthread_mutex_t* mutex;

public:
    print_anticlique_Job(Clique* c, Clique* ac, std::ofstream* out, unsigned int* lc, pthread_mutex_t* m) {
        clique = c;
        anticlique = ac;
        output = out;
        output_lines_counter = lc;
        mutex = m;
    }
    void run() {
        Cliquenode* e = clique->head;
        while(e != NULL) {       // for every entry in c clique
            Cliquenode* d_e = anticlique->head;
            std::string url1 = e->data->get_page_title() + "//" + e->data->get_id();
            while( d_e != NULL ) {          // for every entry in different clique
                std::string url2 = d_e->data->get_page_title() + "//" + d_e->data->get_id();
                pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
                *output << url1 << "," << url2 << ",0" <<  "\n";
                *output_lines_counter += 1;
                pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
                d_e = d_e->next;
            }
            e = e->next;
        }
    }
};

struct JobScheduler{
    int execution_threads;  //number of execution threads
    pthread_t* tids; // execution threads    
    
    //Queue* q;             // a queue that holds submitted jobs / tasks
    // mutex, condition variable, ...

    JobScheduler* initialize_scheduler(int n_execution_threads){
        execution_threads = n_execution_threads;
        tids = new pthread_t[execution_threads];
    };

    int submit_job(JobScheduler* sch, Job* j);
    int execute_all_jobs(JobScheduler* sch);
    int wait_all_tasks_finish(JobScheduler* sch);
    int destroy_scheduler(JobScheduler* sch);

};
#endif