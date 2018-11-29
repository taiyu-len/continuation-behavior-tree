An WIP implementation of behavior trees using continuations designed to be built on
asio

A Behavior is an object that can be called via `void(Continuation)` and
`void()`
These functions are responsible for initializing the state (if there is any)
and calling the continuation with a status once it and its subnodes (if the
exist) are done whatever it is doing, this may happen much later as the
continuation can be posted to an event queue or somewhere else.
the first version sets the continuation, while the second uses the existing or
default continuation.

A Continuation is a object that can be called like `void(Status)`

The internals of a behavior tree are composed of a few types of nodes

Composite nodes with any # of children such as `Select` or `Sequence`, 
Decorator nodes with 1 child such as `Not`, `Succeed`, `Repeat*`
Leaf nodes that represent an actual task.

In regular behavior trees, we would call tick(), that would initialize the
behavior IF it needs to, runs it, and return a Status of
`Success/Running/Failure` to the caller.
In this, we call `run()` which initializes it, runs it, and calls the passed in
continuation with the Status.



TODO:
1. implement more nodes.
2. define behavior concept
3. convert to templates for compile time composition.
4. implement adaptor to have runtime composition.
5. try to ensure tailcalls.
6. get it working with asio features.
7. implement nodes taking executors

