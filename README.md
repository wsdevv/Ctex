# CTEX (PRE-RELEASE STAGE) (RESEARCH/LEARNING PROJECT)
v 0.1.0
alternative matching method to regex, specialized for lexing and tokenization.
(the name is being worked upon)


Upsides:
  - easier to learn/remember syntax
  - easy to implement from scratch (<500 lines of code at its current state)
  - pretty fast, O(n) time complexity for matching/lexing
  - multiple handlers/cases for different matches

Downsides:
  - It's a big memory hog since it utilizes TRIES
  - Missing some functionality that regex's have
  - Could create a regex builder with similar syntax
  - Adds some startup time to the application (as it builds the multiple tries needed to match) (will be fixed with comptime evaluation by 2.0)

syntax example (remember to include ctex.cpp file)
```
    Ctex ctex = Ctex();

    ctex.start()->
        option_then("hello ")-> /* will match "hello " and add it to the result  /  token list
            collect_until_then("world!")-> /* will match everything before "world!", but separate "world!" and everything before it in the result / token list */
                handle_consume([](std::vector<std::string_view>* result) {
                    // do stuff with result (should be a vector of 3)
                })
        .end();

    ctex.walk("hello adsadfasdf world!");
    // should execute the function with result "hello", " asdsadfasdf ", "world!"
```

# TODO
   -  [ X ] Basic matching / tokenization
   -  [ X ] Basic matching any string
   -  [ ] Error handling
   -  [ ] Error Matching
   -  [ ] Multi String Matching/range matching
   -  [ ] More than one handler
   -  [ ] Match all / do not consume/erase array after one match
   -  [ ] option to have "walk" return a token vector instead of using methods
   -  [ ] Fix double free problems
   -  [ ] Code quality improvements
   -  [ ] Documentation
   -  [ ] Compile-time evaluation / memory saving (2.0)
   -  [ ] REGEX builder version
   - ... will be updated with more missing features later
