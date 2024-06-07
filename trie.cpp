#include <cstdlib>
#include <queue>
#include <stack> 
#include <functional> 
#include <cassert>

// TODO: implement removing objects
template<typename T>
class Trie {
  private:
    Trie<T>* level[128] = {0}; // only supports ascii
    std::vector<Trie<T>*> children;
    T leaf;
    int window; // utilized for DFS
    bool head;
    char self;



public:

    // for preventing single-threaded double free, not necessary if you're using TRIE alone'
    // TODO: should probably create an interface or template class to handle this OR use shared_ptr
    // Furthermore, this method is not fullproof since there is a slight chance uninitialized memory after free will be used by another part of a program and be translated to true
    bool is_not_freed = true;

    // for a leaf
    T object;
    // for a leaf
    // should only be used internally!
    std::string* full;

    Trie() {
      this->is_not_freed = true;
      this->self = '\0';
      this->window = 0;
      this->full = 0;
      this->head = true;
      this->object = 0;
   }

   Trie(char self) {
     this->is_not_freed = true;
     this->self = self;
     this->window = 0;
     this->full = 0;
     this->head = false;
     this->object = 0;
   }

   ~Trie<T>() {
     this->is_not_freed = false;
     if (this->full!=0) delete this->full;
     if (this->head) perform_dfs([](Trie* trie) {
       delete trie;
       return false;
    });
   }

   T get() {
     return this->object;
   }

   char get_current_char() {
     return this->self;
   }

   std::string* get_word() {
     return this->full;
   }

   Trie<T>* get_next(char next_char){
     return this->level[next_char%128];
   }

   Trie<T>* set_next(char next_char, T object) {

     if (this->level[(int)next_char%128]==0) {
       Trie<T>* location = new Trie<T>(next_char);
       location->object = object;
       this->level[(int)next_char%128] = location;
       this->children.push_back(location);
    }
    return this->level[(int)next_char%128];


  }

   // TODO: this does not function correctly, make it so it gets a leaf node
   // full search
  // Trie<T>* get(char* str) {
  //   int index = 0;
  //   Trie<T>* current = this;
  //   while(str[index]!='\0') {
  //     if (current->get_next(str[index])==0) return 0;
  //     current = current->get_next(str[index]);
  //     index++;
  //   }
  //   return current;
  // }

   void add(char* str, T object) {

     assert(this->head == true);
     int index = 0;
     std::string* full = new std::string(str);
     Trie* current = this;
     for (int index=0;index<full->size();index++){
       current = current->set_next(str[index], 0);
     }
     current->full = full;
     current->object = object;
   }

   // (note: lambda runs in reverse/after the search hits a leaf node)
   // performs a depth-first scan (without Trie rules) for closest values
   // this is, of course, only to be used after "narrowing down" from the root
   Trie<T>* perform_dfs(std::function<bool(Trie<T>*)> check) {
     Trie* current = this;
     int window = 0;
     std::stack<Trie<T>*> cache;
     cache.push(current);

     while (!cache.empty()) {
       while (!current->children.empty()) {
         current = current->children[current->window];
         cache.push(current);
      }

      while (current->window+1 >= current->children.size()) {
        if (cache.top() == this) return this;
        if (check(cache.top())==true) return cache.top();
        cache.pop();
        current = cache.top();

      };
      current->window++;

    }
    // TODO: this is a placeholder
    return current;

  }

};


