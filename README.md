An WIP implementation of behavior trees using continuations designed to work
with asio

the behavior tree is an object the can be called via `run(fn)` or `run()`
the first version takes a continuation of the parent node, which when the
behavior node is done it works and then returns up the behavior tree.
the second version uses the existing continuation when called again, with the
default being a no-op.

the run function then calls the start() virtual function.

the start function is responsible for setting up the state of the object,  
depending on the type of node, it can do a few things.

- Leaf node
  - Can resume the continuation with some status
    ```c++
    leaf::start() {
      do_stuff();
      return resume(Success);
    };
    ```
  - Can pass the continuation to an executor to be called later, such as a
    waiting for a period of time, or reading a socket, or other async tasks.
    ```c++
    leaf::start() {
      do_stuff();
      return event_queue.post([this]{
        do_stuff_later();
        return resume(Success);
      });
    };
    ```
- Composite nodes
  - creates and passes a continuation to its child which when completed calls
    the parent continuation.
    ```c++
    composite::start() {
      do_stuff();
      return child->run([this](Status s) {
        do_stuff();
        if (s) return child->run(); // run child again with this callback
        else return resume(s);
      });
    };
    ```

The root of a behavior tree can take a final continuation that is called once it
is fully complete. for example, it can delete the behavior tree, or be used
signal the completion of the tree elsewhere, or restart the tree completely.


Unlike regular behavoir trees, there is no `Running` state due to the
continuations, since we simply continue the tree by calling them once we finish
a task.


TODO:
1. [ ] implement more node types.
2. [ ] implement or consider alternate designs
    - [ ] stateless tree by storing state into continuations
    - [ ] templated behavior nodes for one big compile time object
    - [ ] use trait like polymorphic objects to avoid inheritence.
3. [ ] try to ensure tailcalls, or restructure to avoid needing to.
4. [ ] write tests using boost::asio functionality
5. [ ] implement nodes taking executors for better asio compatibility
6. [ ] write a toy server or something similar to test it with a full program.
