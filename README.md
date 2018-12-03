An WIP implementation of behavior trees using continuations designed to work
with asio

the `behavior_t` is a smart pointer wrapping an object satisfying one of these
requirements.
- is invocable as `Status()`
  - used for leaf nodes that only `continues::up` the tree
    ```c++
    behavior_t x = []{ do_stuff(); return Success; };
    ```
- is invocable as `void(continuation)`
  - used for leaf nodes that `continues::elsewhere`, that is it moves the
    continuation to be called later, for async processing
    ```c++
    behavior_t x = [](continuation c)
    {
        timer t(some_time);
        t.async_wait([](...){ do_stuff(); c(); });
    };
    ```
- is invocable as `continues(continuation)`
  - used for nodes which `continues::down` the tree, as well as either of the
    above usages
    ```c++
    node::operator(continuation r) -> continues
    {
        resume_parent = std::move(r);
        resume_this = [this](Status s) -> continues
        {
            // random example
            switch (state()) {
            case 0: return continues::down(child, resume_this);
            case 1: return continues::up(std::move(resume_parent), Success);
            case 2: post(std::move(resume_parent));
                    return continues::elsewhere();
            }
        }
        return continues::down(child, resume_this);
    }
    ```

Unlike regular behavoir trees, there is no `Running` state due to the
continuations, since we simply continue the tree by calling them once we finish
a task.


TODO:
1. [ ] implement more node types.
2. [ ] implement or consider alternate designs
    - [ ] stateless tree by storing state into continuations
    - [ ] templated behavior nodes for one big compile time object
    - [x] use trait like polymorphic objects to avoid inheritence.
3. [x] avoid relying on tail calls
4. [ ] write tests using boost::asio functionality
5. [ ] implement nodes taking executors for better asio compatibility
6. [ ] split into multiple libraries
    - [ ] contrav?
          library for traversing data structures that would have unbounded call
          stack.
    - [ ] contree?
          library built on contrav for continuation based tree
    - [ ] conbtree?
          library built on contree for continuation based behavior trees
8. [ ] write a toy server or something similar to test it with a full program.

