#include <functional>
#include <vector>
#include <string>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <memory>
#include "trie.cpp"

enum CtexMethod {
    NEXT,
    COLLECT_BEFORE,
    COLLECT_BEFORE_EXCLUDED,
    NEXT_EXCLUDED
};

typedef void(*CtexResultHandler)(std::vector<std::string_view>*);
typedef struct CtexResultData {
  CtexMethod method;
  CtexResultHandler handler;
} CtexResultData;

/**
 * @brief C Trie Expression (name is a WIP). Alternative to a Regex for matching tokens and data between tokens. Does not fully compete with regexes, as it is more specialized to token matching
 */
class Ctex { 
  private: 

    // replacable trie
    Trie<Ctex*>* trie;

    // doing this so std::vector doesn't automatically allocate space for every
    // Ctex instance (will only be used for root)
    std::vector<std::string_view>* ctex_cache;

    //std::unordered_map<std::string, Ctex*> map;
    Ctex* root;
    Ctex* last;

    /**
     * TODO: Eventually make this private
     * @param last the Ctex before this new instance
     * @param method the method the current ctex uses
     * @brief Creates a node version of a C Trie Expression, used for matching tokens
     */
    Ctex(Ctex* last, CtexMethod method) {
      this->trie = new Trie<Ctex*>();
      this->last = last;
      this->root = last->root;
      this->ctex_cache = 0;
      this->operation.handler = 0;
      this->operation.method = method;
    }

  public:
  
    CtexResultData operation;

    /**
     * @brief Creates a root version of a C Trie Expression, used for matching tokens
     */
    Ctex() {
      this->trie = new Trie<Ctex*>();
      this->ctex_cache = new std::vector<std::string_view>();
      this->last = this->start();
      this->root = this->start();
      this->operation.method = CtexMethod::NEXT; 
      this->operation.handler = 0;
    } 

    /**
     * @brief creates a child whose trie will be added to another Ctex
     * TODO: the first capture levels "last" feild will be invalid, fix this.
     * @param eventual_root eventual root for this child ctex (must be declared before)
     * @return returns a new ctex on the stack
     */
    static Ctex child(Ctex* eventual_root) {
      Ctex ctex = Ctex();
      ctex.root = eventual_root;
      ctex.operation.method = CtexMethod::NEXT;
      ctex.operation.handler = 0;

      // could be optimized to not create a vector in the first place, but will do later
      delete ctex.ctex_cache;
      ctex.ctex_cache = 0;
      return ctex;
    }

   /**
    * FIXME: Since ctex_cache is a pointer to a object that is naturally a pointer, we may have a memory leak, check this later
    * @brief Ctex destructor, performs a inverse depth-first  search to delete all child Ctex instances, also takes care of the ctex_cache
    */
    ~Ctex() {
      if (this->ctex_cache != 0 ) delete this->ctex_cache;
      if(this->trie->is_not_freed==false) return;
      this->trie->perform_dfs([](Trie<Ctex*>* b){
        if (b->get_word()!=0) {
          delete b->object;
        }
        return false;
      });

      delete this->trie;

    }

    /**
     * @brief Tests if the current ctex is the root ctex
     */
    bool is_root() {
      return this->ctex_cache!=0;
    }

    /**
     * @brief starts the ctex builder syntax (required)
     * @return the current ctex as a pointer
     */
    Ctex* start() {
      this->operation.method = CtexMethod::NEXT;
      return this;
    }

    /**
     * TODO: This function is not fully implemented, please add the non-consume functionality
     * TODO: multiple handlers not implemented
     * @param handler a handler function which accepts a vector of strings (what all previous ctex's captured)
     * @return the current ctex capture group
     * @brief handles whats currently in the ctex capture vector, however, does not consume it and continues on in the current ctex capture group
    */
    Ctex* handle(CtexResultHandler handler) {
      assert(this->operation.handler == 0);
      this->operation.handler = handler;
      return this; 
    }

    /**
     *  @param handler a handler function which accepts a vector of strings (what all previous ctex's captured)
     *  @return the root ctex capture group
    */
    Ctex* handle_consume(CtexResultHandler handler) {
      this->handle(handler);
      return this->return_to_root();
    }

    /**
     * @brief moves back to the parent ctex capture group
    */
    Ctex* end() {
      return this->last;
    }

    /**
     * TODO: add assertions to validate the previous operation
     * @brief an alias to end() to make it more apparent what operation was previously performed
    */
    Ctex* end_collection() { 
      return this->end();  
    }
    
    /**
     * TODO: add assertions to validate the previous operation
     * @brief an alias to end() to make it more apparent what operation was previously performed
    */
    Ctex* end_option() {
      return this->end();
    };

    /**
     * @param option the text to match
     * @return the next ctex matching group or ctex operation to perform after this option is matched, use end() to move back to the parent ctex capture group
     * @brief adds an option with the default ctex method which adds the matched text to the output vector
    */
    Ctex* option_then(char* option) {
      Ctex* creation = new Ctex(this, CtexMethod::NEXT);
      this->trie->add(option, creation);
      return creation;
    };

    /**
     * @brief adds an option and sets a Ctex rule to match text without including the token.
     * @return next ctex
    */
    Ctex* option_excluded(char* option) {
      Ctex* creation = new Ctex(this, CtexMethod::NEXT_EXCLUDED);
      this->trie->add(option, creation);
      return creation;
    }

    /**
     * @brief adds an option and sets the ctex rule to collect/match every character before a string.
     * @return next ctex
    */
    Ctex* collect_until_then(char* option) {
      Ctex* creation = new Ctex(this, CtexMethod::COLLECT_BEFORE);
      this->trie->add(option, creation);
      return creation;
    };

    /**
     * @brief adds an option and sets the ctex rule to collect/match every character before a string, but don't match the string'
     * @return next ctex
     */
    Ctex* collect_until_and_exclude_then(char* option) {
      Ctex* creation = new Ctex(this, CtexMethod::COLLECT_BEFORE_EXCLUDED);
      this->trie->add(option, creation);
      return creation;
    }

    /**
     * @brief returns to the root ctex
     * @return root ctex
    */
    Ctex* return_to_root() {
      return this->root;
    };

    /**
     * @brief overwrites the current ctex with the new one
     * TODO: merge ctex, don't overwrite
     * @param ctex ctex to merge
     */
    Ctex* use_ctex(Ctex* child_ctex) {
      if(this->trie->is_not_freed == true) delete this->trie;
      // TODO: Potential Double free here (fix this)
      this->trie = child_ctex->get_trie();
      return this;
    }


    /**
     * ends the current matching group and jumps down x parent trees
    */
    Ctex* jump_down(int level) {
      Ctex* current = this;
      for (int x=0;x<level;level++) {
        current = current->last; 
      }
      return current;
    };

    /**
     * TODO: fix any memory leaks (documented further within the function)
     * @name walk
     * @param to_match the string to match and extract tokens and information from
     * @return returns nothing, handle functions are necessary
     * @brief "walks" through the inner trie to and adheres to the set ctex rules to match information, adding information and tokens to the root Ctex.
    */
    int walk(std::string_view to_match) {
      Trie<Ctex*>* current_trie = this->trie;
      int index = 0;
      int last_offense = 0;
      //std::cout << to_match << "\n";
      for (char character : to_match) {
        if (index>to_match.size()) break;
        //std::cout << current_trie->get_current_char() << ", " << character << ", " << index << "\n";

        // obeys the method and continues on to the next ctex for the next required match
        if (current_trie->get_word()!=0 && current_trie->get() != 0) {

          //std::cout << "\nfound next: " << current_trie->get_word() << " @ " << index << "\n";
          follow_method(to_match, current_trie, index);
          if (current_trie->get()->operation.handler!=0) {
            //std::cout << "\nexecuting!";
            current_trie->get()->operation.handler(this->root->ctex_cache);
            this->root->ctex_cache->clear();
            return index;
          }
          index += current_trie->get()->walk(to_match.substr(index));
          current_trie = this->root->get_trie();
        }

        Trie<Ctex*>* next = current_trie->get_next(character);
        //std::cout << next->get_current_char() << "\n";

        // TODO: need to prove that this does what it needs to all of the time
        if (next != 0) {
          //std::cout << "getting next...\n";
          current_trie = next;
        }


          index++;
      }

      return index;
    }

    // TODO: make this private
    /**
     * @param to_match the string view use in matching
     * @param current_trie the current trie pointed to in the walk function (should be a ctex leaf node)
     * @param index the current index reached in the matching string
     * @return nothing
     * collects data based on the method provided (operating on a ctex leaf node)
     */
    void follow_method(std::string_view& to_match, Trie<Ctex*>* current_trie, int index) {
        //std::cout << this->root << "\n";
        switch (current_trie->get()->operation.method) {
            case CtexMethod::COLLECT_BEFORE:
              this->root->ctex_cache->push_back(to_match.substr(0, index-current_trie->get_word()->size()));
              this->root->ctex_cache->push_back(std::string_view(current_trie->get_word()->data(), current_trie->get_word()->size()));
              break;
            case CtexMethod::COLLECT_BEFORE_EXCLUDED:
              this->root->ctex_cache->push_back(to_match.substr(0, index-current_trie->get_word()->size()));
              break;
            case CtexMethod::NEXT:
              this->root->ctex_cache->push_back(std::string_view(current_trie->get_word()->data(), current_trie->get_word()->size()));
              break;
            case CtexMethod::NEXT_EXCLUDED:
              break;
            default:
              break;
        };
    }


    /**
     * @brief unsafe operation, should only be used internally
     */
    Trie<Ctex*>* get_trie() {
      return this->trie;
    }

    /**
     * @brief unsafe operation, should only be used internally
     */
    void set_root(Ctex* ctex) {
      this->root = ctex;
    }


}; 
 
