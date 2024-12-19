# Workspaces

Workspaces encapsulate non-const members of corresponding classes. The primary purpose is memory reuse as well as decoupling stateful objects from the context in which they are used, allowing several threads to instantiate their own workspace.

Workspaces must always be a class without any public member, except for the implicitly generated default constructor. Each member should be encapsulated in an optional and there must be a getter that lazy initializes the value in the optional and returns a non-const reference to it.

Workspaces often contain other workspaces to be passed downwards in the call chain.
