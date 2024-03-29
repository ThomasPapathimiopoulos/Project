#ifndef _JOB_SCHEDULER_
#define _JOB_SCHEDULER_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <dirent.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

#include "entry.hpp"
#include "hashtable.hpp"
#include "parser.hpp"
#include "dict.hpp"
#include "logistic_regression.hpp"
#include "sparse_matrix.hpp"
#include "queue.hpp"
#include "double_linked_list.hpp"

class Queue;
class LR;

extern std::mutex print_mutex;
extern std::mutex queue_mutex;
extern std::mutex train_mutex;
extern std::mutex retrain_mutex;
extern std::mutex test_mutex;


//============================================================================================================

class Job{
public:
    virtual void run() = 0;
    virtual ~Job() { }
};

//============================================================================================================

class TestJob : public Job{
private:
    unsigned int id;
public:
    TestJob(unsigned int);
    void run() override;
};

//============================================================================================================

class lr_train_Job : public Job {
    std::string line;
    HashTable* ht;
    SM* files;
    LR* lr;

public: 
    lr_train_Job(std::string, HashTable*, SM*, LR*);
    void run() override;
};

//============================================================================================================

class lr_retrain_Job : public Job {
    Entry* e1;
    Entry* e2;
    SM* files;
    DoubleLinkedList* results;
    LR* lr;
    float threshold;
public: 
    lr_retrain_Job(Entry*, Entry*, SM*, DoubleLinkedList*, LR*, float);
    void run() override;
};

//============================================================================================================

class lr_test_Job : public Job {
    unsigned int *pred_counter, *pred_threshold_counter;
    int e1;
    int e2;
    LR *lr;
    SM *files;
    short int label;
    float threshold;
public: 
    lr_test_Job(unsigned int*, unsigned int*, int, int, LR*, SM*, short int, float);
    void run() override;
};

//============================================================================================================

class JobScheduler{
private:
    int execution_threads;  //number of execution threads
    std::thread* tids; // execution threads    
    // mutex, condition variable, ...

public:
    Queue *q;
    
    JobScheduler(int);
    ~JobScheduler();
    void submit_job(Job*);
    void execute_all_jobs();
    void wait_all_tasks_finish();
};

//============================================================================================================

void thread_f(JobScheduler*);

// class best_words_dict_Job : public Job{
// private:
//     unsigned int best_words_number;
//     Dict* best_words;
//     std::string *all_words_vector;
//     int* best_words_pos_vector;
//     int num_words;
// public:
//     best_words_dict_Job(Dict* n_best_words, unsigned int n_best_words_number, std::string *n_all_words_vector, int* n_best_words_pos_vector, int n_num_words){
//         best_words = n_best_words;
//         best_words_number = n_best_words_number;
//         all_words_vector = n_all_words_vector;
//         best_words_pos_vector = n_best_words_pos_vector;
//         num_words = n_num_words;
//     };
//     void run() override {
//         for(unsigned int i = 0 ; i < best_words_number ; i++){
//             int j = best_words_pos_vector[num_words-1-i]; 
//             best_words->root = best_words->insert(best_words->root, all_words_vector[j], j, i);
//         }   
//     }
// };

// class sort_Job : public Job{
// private:
//     float *all_tfidf_sum_vector;
//     int* best_words_pos_vector;
//     int num_words;
// public:
//     sort_Job(float *n_all_tfidf_sum_vector, int* n_best_words_pos_vector, int n_num_words){
//         all_tfidf_sum_vector = n_all_tfidf_sum_vector;
//         best_words_pos_vector = n_best_words_pos_vector;
//         num_words = n_num_words;
//     };
//     void run() override {
//         for(int i = 0 ; i < num_words ; i++) best_words_pos_vector[i] = i;
//         mergeSort(all_tfidf_sum_vector, best_words_pos_vector, 0, num_words-1);
//     }
// };

// class remove_not_best_Job : public Job{
// private:
//     SM *files;
//     Dict *best_words;

// public:
//     remove_not_best_Job(SM *n_files, Dict *n_best_words){
//         files = n_files;
//         best_words = n_best_words;
//     };
//     void run() override {
//         files->remove_not_best(best_words);
//     }
// };

// class sparse_matrix_Job : public Job{
// private:
//     SM *files;
//     Clique* list_of_entries;
//     Dict *all_words;
//     std::string *all_words_vector;
//     float *all_idf_vector;
//     float *all_tfidf_sum_vector;
//     int j;
//     Wordlist** file_vector;
//     Cliquenode* temp;

// public:
//     sparse_matrix_Job(  Clique* n_list_of_entries, Dict *n_all_words, std::string *n_all_words_vector, float *n_all_idf_vector, 
//                         float *n_all_tfidf_sum_vector, int n_file_pos, Wordlist** n_file_vector, Cliquenode* n_temp){
        
//         list_of_entries = n_list_of_entries;
//         all_words = n_all_words;
//         all_words_vector = n_all_words_vector;
//         all_idf_vector = n_all_idf_vector;
//         all_tfidf_sum_vector = n_all_tfidf_sum_vector;

//         j = n_file_pos;
//         file_vector = n_file_vector;
//         temp = n_temp;
//     };
//     void run() override {
//         Entry* e;
//         std::string word;

//         int word_loc;                           //location of word in all words vector
//         int word_counter = 0;                   //counter for words in entry specs 
        
//         e = temp->data;                         //get the entry
//         e->loc = j;                             //save row number as loc for entry

//         int counter_array[all_words->get_size()] = { 0 };   //init counter array for this entry
//         std::istringstream iss(e->specs_words);
//         while(iss){                                         // for each word in the specs
//             word_counter++;                                 //update word counter 
//             iss >> word;
//             word_loc = all_words->find_loc(all_words->root, word);  //find it's location in the all words vector
//             counter_array[word_loc] = counter_array[word_loc] + 1;  //update parallel location in counter array 
//         }
//         for(unsigned int i = 0 ; i < all_words->get_size() ; i++) {         //go through counter array
//             if(counter_array[i] != 0){                                      //if you find a word that exists in specs
//                 int bow = counter_array[i];                                 //calculate it's bow
//                 float tfidf = (float)bow/(float)word_counter;               //calculate it's tfidf
//                 file_vector[j]->push(all_words_vector[i], i, bow, tfidf);   //add it to the wordlist

//                 all_tfidf_sum_vector[i] = all_tfidf_sum_vector[i] + tfidf;  //increase tfidf sum vector
//             }
//         }
//         temp = temp->next;  
//     }
// };

// class vectorify_Job : public Job{
// private:
//     Dict *all_words;
//     std::string *all_words_vector;
//     float *all_idf_vector;
//     float *all_tfidf_sum_vector;

//     unsigned int *loc;
//     int num_entries;
// public:
//     vectorify_Job(Dict* n_all_words, std::string * n_all_words_vector, float * n_all_idf_vector, float * n_all_tfidf_sum_vector, unsigned int * n_loc, int n_num_entries){
//         all_words = n_all_words;
//         all_words_vector = n_all_words_vector;
//         all_idf_vector = n_all_idf_vector;
//         all_tfidf_sum_vector = n_all_idf_vector;
//         loc = n_loc;
//         num_entries = n_num_entries;
//     }
//     void run() override {
//         all_words->vectorify(all_words->root, all_words_vector, all_idf_vector, all_tfidf_sum_vector, loc, num_entries);
//     }
// };

// class create_database_Job : public Job { 
//     Dict* all_words;
//     Parser* p;
//     dirent *folder;
//     HashTable* ht;
//     Clique* list_of_entries;
//     pthread_mutex_t* mutex;

// public:
//     create_database_Job(Dict* aw, Parser* parser, dirent* dir, HashTable* hash, Clique* loe, pthread_mutex_t* m) {
//         all_words = aw;
//         p = parser;
//         folder = dir;
//         ht = hash;
//         list_of_entries = loe;
//         mutex = m;
//     }
//     void run() override {
//         // std::cout << "folder = " << folder->d_name << std::endl;
//         // for each json use name for constructor
//         std::string file_dir = "./Datasets/2013_camera_specs/";
//         file_dir.append(folder->d_name); //create every folders path
//         // std::cout << file_dir << "\n";
//         struct dirent *file;
//         Entry* e;
//         DIR *dir_f;
//         if ( (dir_f = opendir(file_dir.c_str())) == NULL ) { //open the created folder path to access jsons
//             perror("can't open the given directory");
//             exit(2);
//         } else {
//             while ( (file = readdir(dir_f)) ) {
//                 if (file->d_name != std::string(".") && file->d_name != std::string("..")) {
//                     // std::cout << "folder = " << folder->d_name << "file = " << file->d_name << std::endl;
//                     std::string id_str = file->d_name; //keep id of product from file's title
//                     size_t lastdot = id_str.find_last_of(".");

//                     std::string id = id_str.substr(0, lastdot);
//                     // std::cout << "folder = " << folder->d_name << " file = " << id_str << std::endl;
//                     // call entry constructor and insert to entry_list and hashtable
//                     std::string path = file_dir+"/"+id_str;

//                     pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
//                     e = new Entry(std::string(folder->d_name), id, p->parse(path));
//                     e->specs_words = e->get_specs()->clean_up();
//                     // std::cout << counter << std::endl;
//                     // counter++;
//                     all_words->root = all_words->add(all_words->root, e->specs_words, &(e->specs_words));
//                     // e->get_specs()->print();
//                     list_of_entries->push(e);
//                     ht->insert(e);
//                     pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
//                 }
//             }

//             (void) closedir (dir_f);
//         }
//     }
// };

// class print_clique_Job : public Job {
//     std::ofstream* output;
//     unsigned int* output_lines_counter;
//     Clique* clique;
//     pthread_mutex_t* mutex;

// public:
//     print_clique_Job(Clique* c, std::ofstream* out, unsigned int* lc, pthread_mutex_t* m) {
//         clique = c;
//         output = out;
//         output_lines_counter = lc;
//         mutex = m;
//     }
//     void run() override {
//         int size = clique->size; 
//         Cliquenode* table[size];            //create table of clique's members
//         Cliquenode* temp_entry = clique->head;
//         for(int i=0 ; i < size ; i++) {
//             table[i] = temp_entry;
//             temp_entry = temp_entry->next;
//         }

//         // print every possible pair from that clique
//         for(int i=0 ; i<size ; i++) {       
//             std::string url1 = table[i]->data->get_page_title() + "//" + table[i]->data->get_id();
//             for(int j=i+1 ; j<size ; j++) {
//                 std::string url2 = table[j]->data->get_page_title() + "//" + table[j]->data->get_id();
//                 pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
//                 *output << url1 << "," << url2 << ",1" <<  "\n";
//                 *output_lines_counter += 1;
//                 pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
//             }
//             // table[i]->data->clique = NULL;  //make clique pointer NULL for all those entries so we don't print any pair more than once
//         }
//     }
// };

// class print_anticlique_Job : public Job {
//     std::ofstream* output;
//     unsigned int* output_lines_counter;
//     Clique* clique;
//     Clique* anticlique;
//     pthread_mutex_t* mutex;

// public:
//     print_anticlique_Job(Clique* c, Clique* ac, std::ofstream* out, unsigned int* lc, pthread_mutex_t* m) {
//         clique = c;
//         anticlique = ac;
//         output = out;
//         output_lines_counter = lc;
//         mutex = m;
//     }
//     void run() override {
//         Cliquenode* e = clique->head;
//         while(e != NULL) {       // for every entry in c clique
//             Cliquenode* d_e = anticlique->head;
//             std::string url1 = e->data->get_page_title() + "//" + e->data->get_id();
//             while( d_e != NULL ) {          // for every entry in different clique
//                 std::string url2 = d_e->data->get_page_title() + "//" + d_e->data->get_id();
//                 pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
//                 *output << url1 << "," << url2 << ",0" <<  "\n";
//                 *output_lines_counter += 1;
//                 pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
//                 d_e = d_e->next;
//             }
//             e = e->next;
//         }
//     }
// };

// class read_output_Job : public Job {
//     unsigned int i;
//     std::ifstream* input;
//     std::string *train_set;
//     pthread_mutex_t* mutex;

// public:
//     read_output_Job(std::string* ts, std::ifstream* in, unsigned int iter, pthread_mutex_t* m) {
//         train_set = ts;
//         input = in;
//         i = iter;
//         mutex = m;
//     }
//     void run() override {
//         pthread_mutex_lock(mutex); // ADD NEEDED MUTEX
//         getline(*input, train_set[i]);
//         pthread_mutex_unlock(mutex); // ADD NEEDED MUTEX
//     }
// };
#endif