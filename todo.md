1. [ ] implement more node types.
2. [ ] implement or consider alternate designs
    - [ ] stateless tree by storing state into continuations
    - [ ] templated behavior nodes for one big compile time object
    - [x] use trait like polymorphic objects to avoid inheritence.
3. [ ] write tests using boost::asio functionality
4. [ ] implement nodes taking executors for better asio compatibility
5. [ ] split into multiple libraries
    - [ ] contrav?
          library for traversing data structures that would have unbounded call
          stack.
    - [ ] contree?
          library built on contrav for continuation based tree
    - [ ] conbtree?
          library built on contree for continuation based behavior trees
7. [ ] write a toy server or something similar to test it with a full program.
8. [ ] optimize sequence nodes with immutable number of children
9. [ ] add support for allocators
10. [ ] use better status type
    - [ ] have thrown exceptions turn into aborted status codes.
