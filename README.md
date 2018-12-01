An WIP implementation of behavior trees using continuations designed to work
with asio

the `behavior_t` object is smart pointer wrapping an object satisfying the
`behavior` concept.
the `behavior_t` object can be called via `bt(fn)` or `bt()`
1. takes a continuation which resumes the calling function, eventually called
   via `fn(Success)` or `fn(Failure)`
2. uses the existing continuation when called again. the default being a no-op.

the `behavior` concept is any object that can be called via `bt(fn)` or
`Status(bt())`.
1. taking the continuation of the calling function.
2. returns a Status which is passed into a continutation directly
it function call is responsible for setting up the state of the behavior,
and depending on the type may do a few things
- Leaf node
  - Can resume the continuation with some status
    ```c++
    leaf::operator()(resume&) {
      do_stuff();
      return resume(Success);
    };
    // OR
    leaf::operator()() -> Status {
      do_stuff();
      return Success; // or Failure
    }
    ```
  - Can pass the continuation to an executor to be called later, such as a
    waiting for a period of time, or reading a socket, or other async tasks.
    ```c++
    leaf::operator()(resume&) {
      do_stuff();
      return event_queue.post([this, &] {
        do_stuff_later();
        return resume(Success);
      });
    };
    ```
- Branch nodes
  - creates and passes a continuation to its child which when completed calls
    the parent continuation.
    ```c++
    composite::operator()(resume&) {
      do_stuff();
      return child([this, &](Status s) {
        do_stuff();
        if (s) return child(); // run child again with this callback
        else return resume(s); // resume the calling function
      });
    };
    ```
    - an example for when you cannot reuse continuations
    ```c++
    composite::operator()(resume&) {
      do_stuff();
      next(resume);
    }
    composite::next(resume&) {
      return child[index]([this, &](Status s) {
        do_stuff();
        if (s) return next(resume); // run next child again with this callback
        else return resume(s); // resume the calling function
      });
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
3. [ ] try to ensure tailcalls, or restructure to avoid needing to.
4. [ ] write tests using boost::asio functionality
5. [ ] implement nodes taking executors for better asio compatibility
6. [ ] write a toy server or something similar to test it with a full program.
