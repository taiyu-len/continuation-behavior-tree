An WIP implementation of behavior trees using continuations designed to work
with asio

the `behavior` is a smart pointer wrapping an object satisfying one of these
requirements.
- is invocable as `status()`
  - used for leaf nodes that only `continues::up` the tree
    ```c++
    behavior x = []{ do_stuff(); return status::success; };
    ```
- is invocable as `void(continuation)`
  - used for leaf nodes that `continues::elsewhere`, that is it moves the
    continuation to be called later, for async processing
    ```c++
    behavior x = [](continuation c)
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
        resume_this = [this](status s) -> continues
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
