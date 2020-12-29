#include "utility.hpp"

unsigned long long hash_value_calculator(std::string page_title, std::string id) {
    //generating hashvalue for entry
    unsigned long long hashvalue = 5381;

    int c;
    const char* site_c = page_title.c_str();
    const char* id_c = id.c_str();

    while ( (c = *site_c++) )
        hashvalue = ((hashvalue << 5) + hashvalue) + c; /* hash * 33 + c */

    while ( (c = *id_c++) )
        hashvalue = ((hashvalue << 5) + hashvalue) + c; /* hash * 33 + c */

    return hashvalue;
}

// fills up bow anf tf_idf tables
void create_bow_and_tf(int** bow, float** tf_idf, Clique* list_of_entries, Dict* dictionary) {
    Cliquenode* temp = list_of_entries->head;
    Entry* e;
    std::string word;
    // int bow[list_of_entries->get_size()][dictionary_size];
    // int tf_idf[list_of_entries->get_size()][dictionary_size];
    int word_loc;
    int entry_counter = 0;
    int word_counter = 0;                           // keeps total number of description's words 
    // for each entry
    while(temp != NULL) {
        e = temp->data;
        e->loc = entry_counter;                     // save its row number of arrays
        std::istringstream iss(e->specs_words);
        // for each word in its description
        while(iss) {
            word_counter++; 
            iss >> word;
            word_loc = dictionary->find_loc(dictionary->root, word);
            if( word_loc != -1 ) {                  // if word exists in dictionary
                bow[entry_counter][word_loc] = bow[entry_counter][word_loc] + 1;  // increase its value in bow table
            }
        }
        // after finishing bow table
        // run through it and create tf values 
        for(int i=0 ; i < DICTIONARY_SIZE ; i++) {
            tf_idf[entry_counter][i] = (float)bow[entry_counter][i]/(float)word_counter;
            // if(tf_idf[entry_counter][i] != 0) {
            //     std::cout << tf_idf[entry_counter][i] << "\n";
            // }
        }

        temp = temp->next;
        entry_counter++;
    }
}

// -----------------SORT-------------------------

void merge(float* tfidf, int* words, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
 
    float L[n1], R[n2];
    int Ls[n1], Rs[n2];
 
    for (int i = 0; i < n1; i++) {
        L[i] = tfidf[l + i];
        Ls[i] = words[l+i];
    }
    for (int j = 0; j < n2; j++) {
        R[j] = tfidf[m + 1 + j];
        Rs[j] = words[m + 1 + j];
    }
 
    int i = 0;
    int j = 0;
    int k = l;
 
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            tfidf[k] = L[i];
            words[k] = Ls[i];
            i++;
        }
        else {
            tfidf[k] = R[j];
            words[k] = Rs[j];
            j++;
        }
        k++;
    }
 
    while (i < n1) {
        tfidf[k] = L[i];
        words[k] = Ls[i];
        i++;
        k++;
    }
 
    while (j < n2) {
        tfidf[k] = R[j];
        words[k] = Rs[j];
        j++;
        k++;
    }
}

void mergeSort(float* tfidf, int* words, int l, int r) {
    if(l>=r) {
        return;//returns recursively
    }
    int m = (l+r-1)/2;
    mergeSort(tfidf, words, l, m);
    mergeSort(tfidf, words, m+1, r);
    merge(tfidf, words, l, m, r);
}

// -----------------SHUFFLE-------------------------

void swap(std::string* a, std::string* b) {
    std::string temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

void shuffle(std::string* array, unsigned int size) {
    srand(time(NULL));
    // int s1,s2;
    for(unsigned int i=0 ; i < size ; ++i) {
        // s1 = rand()%size;
        // s2 = rand()%size;
        swap(array[i], array[rand()%size]);
    }
}