#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <thread>
#include <mutex>

#include <dirent.h>
#include "entry.hpp"
#include "hashtable.hpp"
#include "parser.hpp"
#include "dict.hpp"
#include "logistic_regression.hpp"
#include "sparse_matrix.hpp"
#include "job_scheduler.hpp"
#include "double_linked_list.hpp"
#include <bits/stdc++.h> 
#include <sys/time.h> 

int main() {
    unsigned int thread_num = 4;
    unsigned int job_batch_size = 30000000;
    unsigned int train_iters = 3;
    unsigned int epochs = 5;
    float threshold = 0.1;
    float learning_rate = 5;
    unsigned int batch_size = 4096;
    float job_threshold = 0.025;
    float job_threshold_step = 0.005;
    unsigned int best_words_number = 500;
    
    
    std::ifstream config;
    config.open ("config.ini");

    std::string config_line;
    while(getline(config, config_line)){
        std::stringstream config_iss(config_line);
        std::string word;
        getline(config_iss, word, ' ');
        if(word == "thread_num"){
            getline(config_iss, word, ' ');
            thread_num = std::stoi(word);
        }
        else if(word == "job_batch_size"){
            getline(config_iss, word, ' ');
            job_batch_size = std::stoi(word);
        }
        else if(word == "train_iters"){
            getline(config_iss, word, ' ');
            train_iters = std::stoi(word);
        }
        else if(word == "epochs"){
            getline(config_iss, word, ' ');
            epochs = std::stoi(word);
        }
        else if(word == "threshold"){
            getline(config_iss, word, ' ');
            std::string::size_type sz = strlen(word.c_str());
            threshold = std::stof(word, &sz);
        }
        else if(word == "learning_rate"){
            getline(config_iss, word, ' ');
            std::string::size_type sz = strlen(word.c_str());
            learning_rate = std::stof(word, &sz);
        }
        else if(word == "batch_size"){
            getline(config_iss, word, ' ');
            batch_size = std::stoi(word);
        }
        else if(word == "job_threshold"){
            getline(config_iss, word, ' ');
            std::string::size_type sz = strlen(word.c_str());
            job_threshold = std::stof(word, &sz);
        }
        else if(word == "job_threshold_step"){
            getline(config_iss, word, ' ');
            std::string::size_type sz = strlen(word.c_str());
            job_threshold_step = std::stof(word, &sz);
        }
        else if(word == "best_words_number"){
            getline(config_iss, word, ' ');
            best_words_number = std::stoi(word);
        }
    }

    std::cout << "learning rate = " << learning_rate << " | words number = " << best_words_number << " | batch size = " << batch_size << 
        " | thread num = " << thread_num << " | job batch size = " << job_batch_size << " | train iters = " << train_iters <<
        " | initial retrain threshold = " << job_threshold << " | step = " << job_threshold_step << std::endl;

    std::cout << "Starting..." << std::endl;
    
    JobScheduler js(thread_num);
    struct timeval start, end;
    
    DIR *dir_p;
    DIR *dir_f;
    struct dirent *folder;
    HashTable ht(100);
    Clique list_of_entries;
    Entry* e;
    std::string input_dir = "./Datasets/2013_camera_specs/";
    Parser p;
    Dict all_words;
    unsigned int json_counter = 0;
    // int counter = 0;
    if ( (dir_p = opendir(input_dir.c_str())) == NULL ) {
        perror("can't open the given directory");
        exit(2);
    } else {
        std::cout << "Reading the datasets..." << std::flush;
        while ( (folder = readdir(dir_p)) ) { //read each folder
            if (folder->d_name != std::string(".") && folder->d_name != std::string("..")) {
                // NOTE: REPLACE WITH JOB SCHEDULE ADD 

                // std::cout << "folder = " << folder->d_name << std::endl;
                // for each json use name for constructor
                std::string file_dir = "./Datasets/2013_camera_specs/";
                file_dir.append(folder->d_name); //create every folders path
                // std::cout << file_dir << "\n";
                struct dirent *file;
                if ( (dir_f = opendir(file_dir.c_str())) == NULL ) { //open the created folder path to access jsons
                    perror("can't open the given directory");
                    exit(2);
                } else {
                    while ( (file = readdir(dir_f)) ) {
                        if (file->d_name != std::string(".") && file->d_name != std::string("..")) {
                            json_counter++;
                            // std::cout << "folder = " << folder->d_name << "file = " << file->d_name << std::endl;
                            std::string id_str = file->d_name; //keep id of product from file's title
                            size_t lastdot = id_str.find_last_of(".");
                            if (lastdot == std::string::npos) {
                                // call entry constructor and insert to entry_list and hashtable
                                e = new Entry(std::string(folder->d_name), id_str);
                                list_of_entries.push(e);
                                ht.insert(e);
                            } else {
                                std::string id = id_str.substr(0, lastdot);
                                // std::cout << "folder = " << folder->d_name << " file = " << id_str << std::endl;
                                // call entry constructor and insert to entry_list and hashtable
                                std::string path = file_dir+"/"+id_str;
                                e = new Entry(std::string(folder->d_name), id, p.parse(path));
                                e->specs_words = e->get_specs()->clean_up();
                                // std::cout << counter << std::endl;
                                // counter++;
                                all_words.root = all_words.add(all_words.root, e->specs_words, &(e->specs_words));
                                // e->get_specs()->print();
                                list_of_entries.push(e);
                                ht.insert(e);
                            }
                        }
                    }

                    (void) closedir (dir_f);
                }
            }

            // NOTE: RUN TASKS 
        }
        std::cout << "\t\t\t\t\t\t\t\033[1;32mFINISHED\033[0m" << std::endl;        
        (void) closedir (dir_p);
    } 

    // parse csv
    std::ifstream file("./Datasets/sigmod_large_labelled_dataset.csv");
    std::string line, word = "";

    unsigned int lines_counter = 0;
    unsigned int train_size;
    unsigned int test_size;
    // unsigned int counter = 0;
    std::string* dataset;
    std::string* cliques_set;
    std::string* test_set;
    std::string* validation_set;
    if(file.is_open()) {
        while( getline(file, line) ) {
            lines_counter++;
        }
        lines_counter--;
        dataset = new std::string[lines_counter];
        file.clear();
        file.seekg(0);
        std::string empty;
        getline(file, empty);
        for(unsigned int i = 0 ; i < lines_counter ; i++) {
            getline(file, dataset[i]);
        }
        train_size = 0.6*lines_counter;
        test_size = 0.2*lines_counter;
        if( lines_counter > train_size + 2*test_size)
            train_size += lines_counter-(train_size + 2*test_size);
        cliques_set = new std::string[train_size];
        test_set = new std::string[test_size];
        validation_set = new std::string[test_size];
        shuffle(dataset, lines_counter);
        for(unsigned int i=0 ; i < lines_counter ; ++i) {
            if(i < train_size)
                cliques_set[i] = dataset[i];
            else if(i < train_size+test_size)
                test_set[i-train_size] = dataset[i];
            else 
                validation_set[i-(train_size+test_size)] = dataset[i];
        }
        delete[] dataset;
        for(unsigned int i = 0 ; i < train_size ; i++) {
            std::stringstream line_stringstream(cliques_set[i]);
            Entry* a = NULL;
            Entry* b = NULL;
            while( getline( line_stringstream, word, ',') ) { //tokenize with delimeter: ","
                // std::cout << word << "\n";
                size_t first_slash = word.find_first_of('/'); //find '/' and strip them
                if ( first_slash == std::string::npos) { //then it's 0 || 1 for similarities
                    if(a != NULL && b != NULL){ //if both specs have been iterated
                        // std::cout << "Merging:" << std::endl;
                        // a->clique->print();
                        // b->clique->print();
                        a->conn_tree->root = a->conn_tree->insert(a->conn_tree->root, b);
                        b->conn_tree->root = b->conn_tree->insert(b->conn_tree->root, a);
                        
                        if( word == "1" ) {
                            a->merge(b);    //merge their cliques
                        }
                        else if( word == "0" ) {
                            a->differs_from(b);
                        }
                    }
                } else { // then it's a products url
                    std::string site = word.substr(0,first_slash);
                    std::string id = word.substr(first_slash+2);
                    // std::cout << site << " " << id << "\n";
                    unsigned long long hash_value = hash_value_calculator(site, id);
                    if(a == NULL) {
                        a = ht.search(hash_value);  //find this entry in hashtable 
                        // std::cout<<"a: ";
                        // a->print();
                        // std::cout<<std::endl;
                        // a->clique->print();
                    }
                    else{
                        b = ht.search(hash_value);  //find this entry in hashtable 
                        // std::cout<<"b: ";
                        // b->print();
                        // std::cout<<std::endl;
                        // b->clique->print();
                    } 
                }

            }

        }
        delete[] cliques_set;
        file.close();

    } else {
        perror("could not open dataset W");
    }
    
    std::cout << "Full dictionary contains " << all_words.get_size() << " unique words." << std::endl;
    int num_entries = list_of_entries.size;
    int num_words = all_words.get_size();
    std::string all_words_vector[num_words];
    float all_idf_vector[num_words];
    float all_tfidf_sum_vector[num_words];

    std::cout << "Vectorifying the full dictionary..." << std::flush;
    unsigned int loc = 0;
    
    all_words.vectorify(all_words.root, all_words_vector, all_idf_vector, all_tfidf_sum_vector, &loc, num_entries);
    std::cout << "\t\t\t\t\t\033[1;32mFINISHED\033[0m" << std::endl;

    std::cout << "Generating bow and tdifd sparse matrices for all words..." << std::flush;
    SM files(&list_of_entries, all_words_vector, all_tfidf_sum_vector, all_idf_vector, &all_words);
    std::cout << "\t\t\033[1;32mFINISHED\033[0m" << std::endl;
   
    std::cout << "Sorting words based on tfidf sums..." << std::flush;
    int best_words_pos_vector[num_words];
    for(int i = 0 ; i < num_words ; i++) best_words_pos_vector[i] = i;
    mergeSort(all_tfidf_sum_vector, best_words_pos_vector, 0, num_words-1);
    std::cout << "\t\t\t\t\t\033[1;32mFINISHED\033[0m" << std::endl;

    std::cout << "Inserting best words in a dictionary..." << std::flush;

    Dict* best_words = new Dict();
    for(unsigned int i = 0 ; i < best_words_number ; i++){
        int j = best_words_pos_vector[num_words-1-i]; 
        best_words->root = best_words->insert(best_words->root, all_words_vector[j], j, i);
    }
    std::cout << "\t\t\t\t\t\033[1;32mFINISHED\033[0m" << std::endl;

    std::cout << "Removing words that aren't best from sparse matrices..." << std::flush;
    files.remove_not_best(best_words);
    delete best_words;
    std::cout << "\t\t\t\033[1;32mFINISHED\033[0m" << std::endl;

    // output printing
    unsigned int output_lines_counter = 0;
    remove( "output.csv" );
    print_output(&list_of_entries, &output_lines_counter);
    // creatin train_set
    std::string* train_set = new std::string[output_lines_counter];
    create_train_set(train_set, output_lines_counter);

    // Creating array of all entries
    Cliquenode* temp = list_of_entries.head;
    unsigned int size = list_of_entries.size;
    Entry** entries_array = new Entry*[size];
    for(unsigned int i=0 ; i < size ; ++i) {
        entries_array[i] = temp->data;
        temp = temp->next;
    }

    LR* lr = new LR(best_words_number*2);

    Entry *e1, *e2;
    DoubleLinkedList* results = new DoubleLinkedList();
    DoubleLinkedNode** results_array;
    gettimeofday(&start, NULL);
    for(unsigned int i=0 ; i < train_iters ; ++i) {
        lr->train(&files, train_set, output_lines_counter, &ht, &js, batch_size, epochs, learning_rate);
        if( i < train_iters-1 ) {
            for(unsigned int j=0 ; j < size ; ++j) {
                e1 = entries_array[j];
                for(unsigned int k=j+1 ; k < size ; ++k) {
                    e2 = entries_array[k];
                    if( !e1->conn_tree->find(e1->conn_tree->root, e2) ) {
                        js.submit_job(new lr_retrain_Job(e1, e2, &files, results, lr, job_threshold));
                        if( (j*size+k) % job_batch_size == 0) {
                            js.execute_all_jobs();
                            js.wait_all_tasks_finish();
                            // std::cout << "Pairs checked " << j << " Results added: " << results->size << std::endl;
                        }
                    }
                }
            }
            if( js.q->size > 0 ) {
                js.execute_all_jobs();
                js.wait_all_tasks_finish();
            }
            std::cout << "Etries to add: " << results->size << std::endl;
            results_array = new DoubleLinkedNode*[results->size];
            unsigned int results_size = results->size;
            for(unsigned int j=0 ; j < results_size ; ++j) {
                results_array[j] = results->pop();
            }
            prediction_mergeSort(results_array, 0, results_size-1);
            for(unsigned int j=0 ; j < results_size ; ++j) {
                e1 = results_array[j]->A;
                e2 = results_array[j]->B;
                if( !(e1->clique->different->find(e2->clique)) && !(e1->clique->find(e2)) ) {
                    if( results_array[j]->pred > 0.5 ) {
                        e1->merge(e2);
                        e1->conn_tree->insert(e1->conn_tree->root, e2);
                        e2->conn_tree->insert(e2->conn_tree->root, e1);
                    } else {
                        e1->differs_from(e2);
                        e1->conn_tree->insert(e1->conn_tree->root, e2);
                        e2->conn_tree->insert(e2->conn_tree->root, e1);

                    }
                }
            }
            delete[] results_array;
            // print output
            remove( "output.csv" );
            clear_print(&list_of_entries);
            print_output(&list_of_entries, &output_lines_counter);
            // create new train set by reading output
            delete[] train_set;
            train_set = new std::string[output_lines_counter];
            create_train_set(train_set, output_lines_counter);
        }
        // increment threshold
        job_threshold += job_threshold_step;
    }
    gettimeofday(&end, NULL);

    lr->predict(&files, test_set, test_size, &ht, &js, threshold);
    lr->validate(&files, validation_set, test_size, &ht, &js, threshold);

    remove( "cliques.csv" );
    clear_print(&list_of_entries);
    print_cliques(&list_of_entries, &output_lines_counter);

    std::cout << "learning rate = " << learning_rate << " | words number = " << best_words_number << " | batch size = " << batch_size << 
        " | thread num = " << thread_num << " | job batch size = " << job_batch_size << " | train iters = " << train_iters <<
        " | initial retrain threshold = " << job_threshold << " | step = " << job_threshold_step << std::endl;

    double time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6; 
    std::cout << "Time taken by training : " << std::fixed << time_taken << std::setprecision(6) << " sec " << std::endl; 

    // empty heap
    delete results;
    delete lr;
    delete[] train_set;
    delete[] test_set;
    delete[] validation_set;
    return 0;
}