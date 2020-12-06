#include "anticlique.hpp"

//-------------------AntiCliquenode-------------------

//AntiCliquenode constructor
AntiCliquenode::AntiCliquenode(Clique* c) {
    data = c;
    next = NULL;
}

//AntiCliquenode destructor, doesnt destroy the clique it points to just unlinks it
AntiCliquenode::~AntiCliquenode(){
    data = NULL;
    next = NULL;
}

//-------------------AntiClique-------------------

//AntiClique constructor
AntiClique::AntiClique(){
    head = NULL;
    size = 0;
}

//AntiClique destructor
AntiClique::~AntiClique(){    
    while(head != NULL){
        AntiCliquenode* temp = head;
        head = head->next;
        delete temp;
    }
    // std::cout << "AntiClique deleted!" << std::endl;
}

//size getter
int AntiClique::get_size(){
    return size;
}

//returns true if AntiClique lsit is empty
bool AntiClique::is_empty(){
    return size == 0;
}

//adds clique to the head of the AntiClique
void AntiClique::push(Clique* c){
    AntiCliquenode *n = new AntiCliquenode(c);
    n->next = head;        
    head = n;
    size++;   
}


//removes clique at the top of the list
AntiCliquenode* AntiClique::pop() {
    AntiCliquenode* temp;
    temp = head;
    head = head->next;
    size--;
    return temp;
}

//merges this AntiClique list with the one given
//by popping and pushing the nodes if the clique is not already there
//deletes the other list
void AntiClique::merge(AntiClique* c){
    while( !(c->is_empty()) ) {
        AntiCliquenode* new_node = c->pop();
        if( !find(new_node->data) ) {
            this->push(new_node->data);
        }
        delete new_node;
    }
    delete c;
}

//returns true if the clique exists somewhere in the AntiClique
//false otherwise
bool AntiClique::find(Clique* c){
    AntiCliquenode* temp = head;
    while(temp != NULL){
        if(c == temp->data){
            return true;
        }
        temp = temp->next;
    }
    return false;
}