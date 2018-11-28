An WIP implementation of behavior trees using continuations designed to be built on
asio

The Behavior tree is simply a Handler, which can be posted to an Executor.

The internals of a behavior tree are composed of a few types of nodes

Composite nodes with any # of children such as `Select` or `Sequence`, 
Decorator nodes with 1 child such as `Not`, `Succeed`, `Repeat*`
Leaf nodes that represent an actual task.

In regular behavior trees, we would call tick(), that would initialize the
behavior IF it needs to, runs it, and return a Status of
`Success/Running/Failure` to the caller.
In this, we call `run()` which initializes it, runs it, and calls the passed in
continuation with the Status.



