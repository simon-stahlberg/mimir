# Mimir Python API

Mimir 0.14.0b3 provides a typed, semantic API for loading PDDL and prototyping
search algorithms.

```python
from pymimir import LiftedFFHeuristic, Problem, SearchStatus, astar

problem = Problem.from_files("domain.pddl", "problem.pddl")
heuristic = LiftedFFHeuristic(problem)
result = astar(problem, heuristic, timeout_seconds=30)

if result.solution is not None:
    for action in result.solution.plan:
        print(action, action.cost)
```

Use `problem.fact("at", "robot", "room")`, `problem.action(...)`, and
`problem.state(...)` to construct values without editing PDDL files. Search
callbacks receive immutable `Transition` records. The `pymimir.advanced`
module is an unstable implementation detail.

## Programmatic construction

`DomainBuilder` and `ProblemBuilder` construct the same native models without
parsing PDDL text. Builder sections follow PDDL order, are single-use, and must
be closed explicitly.

```python
from pymimir import DomainBuilder, ProblemBuilder

domain = (
    DomainBuilder("switches")
    .requirements().add(":strips").close()
    .predicates().add("on", ("?switch", "object")).close()
    .actions()
        .add("turn-on")
        .add_parameter("?switch")
        .add_effect("on", "?switch")
        .close()
        .close()
    .build()
)
problem = (
    ProblemBuilder(domain, "one-switch")
    .objects().add("light").close()
    .goal().add("on", "light").close()
    .build()
)
```

## Learning encodings

`pymimir.learning` provides framework-independent native graph encoding. One
`EncodingContext` owns node IDs, relation rows, and example metadata for a
complete batch. Each example assigns its canonical problem objects first;
encoders then allocate additional nodes in call order.

```python
from pymimir import Domain, Problem, learning

domain = Domain.from_file("domain.pddl")
problem = Problem.from_file(domain, "problem.pddl")
state = problem.initial_state
actions = state.applicable_actions()
successors = [action.apply(state) for action in actions]

with learning.EncodingContext() as context:
    context.begin_instance(problem)
    learning.encode_state(context, state)
    learning.encode_goal(context, state, problem.goal)
    learning.encode_action_list(context, state, actions)
    learning.encode_transition_effects(
        context, state, successors, [], problem.goal
    )
    context.end_instance()

    relation_buffer = context.to_relation_buffer()
    node_sizes = context.node_sizes

relations = relation_buffer.to_relations()
assert node_sizes == [len(problem.all_objects) + 2 * len(actions)]
```

`to_relation_buffer()` performs one native bulk copy into a Python-owned packed
int32 snapshot. Its immutable descriptors use int32-value offsets, and its
writable `values` memoryview can be passed directly to `torch.frombuffer` by
framework integrations. The view and any tensor created from it share storage,
so mutations are visible through both. The snapshot remains valid after the
context closes. `RelationBuffer.to_relations()` and the convenience
`EncodingContext.to_relations()` materialize the compatible
`dict[str, list[int]]` form; those values already contain batch offsets and can
be consumed without PyTorch. Relation row order is unspecified, while argument
order inside each row is preserved.

True nullary predicates are lifted over the active problem's canonical
`all_objects`: state, goal, and expressive encodings emit `P(object)` once for
every object, including domain constants. Transition effects instead emit the
unary relation `P(transition)`. If a problem has no objects, state, goal, and
expressive encodings retain the corresponding relation key with an empty value
buffer, while transition effects still encode their transition node.

Encoding functions append into the native context and return `None`. A batch
may mix Problems only when they share the exact `Domain` object. The context
manager calls the idempotent `close()` method to release the native owner
deterministically; copied buffers, relation dictionaries, and metadata remain
valid after the context is closed. Transition-effect encoding computes fluent
and derived net changes directly from the source and each ordered successor;
every successor, including a no-op or repeated state, receives its own node.

Run the complete example with:

```console
python python/examples/quickstart.py domain.pddl problem.pddl
```
