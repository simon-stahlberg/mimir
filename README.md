# Mimir Python API

Mimir 0.14.0b4 provides a typed, semantic API for loading PDDL and prototyping
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

For Python search loops, `state.successor_states()` returns an immutable tuple
of `(action, successor_state)` pairs. Generation and application run in native
code, and handles cross the boundary in bulk. Pair order matches
`state.applicable_actions()` regardless of which method is called first;
different actions with equal successors remain separate entries. Applicable
actions are cached on the state, while successor states belong to the caller.

```python
for action, successor in problem.initial_state.successor_states():
    print(action, successor)
```

## Batched heuristics and Q-GBFS

State heuristics provide `evaluate_batch(states, goal=None)`. Its default
implementation evaluates each state; native built-ins use one bulk native call.
Existing BFS, UCS, A*, GBFS, and IW searches retain their scalar evaluation paths.

`QHeuristic` scores an entire expansion: a state and an ordered sequence of
`(action, successor_state)` tuples. Override `evaluate_batch` for batched models;
the default calls `evaluate` once per parent. Each output row must have exactly
one score per supplied transition. The inputs include already-generated
successors and preserve complete action rows, including duplicate and previously
visited successors. Python callbacks may retain these objects after returning.

```python
import pymimir

class SuccessorGoalCount(pymimir.QHeuristic):
    def __init__(self, problem):
        super().__init__(problem)
        self.heuristic = pymimir.GoalCountHeuristic(problem)

    def evaluate(self, state, successors, goal=None):
        return self.heuristic.evaluate_batch(
            [successor for action, successor in successors], goal
        )

result = pymimir.qgbfs(
    problem,
    SuccessorGoalCount(problem),
    maximize=False,
    batch_target=32,
)
```

`batch_target` is a soft target for newly reached states per search batch.
Complete parent expansions are never split. The evaluator controls any model
batch-size limit internally. Q-GBFS improves the incoming score and path of
states still in the frontier, preserving their original discovery order for
FIFO ties. Expanded states stay closed. Every evaluated row retains all of its
successors, including duplicates and closed states, and all scores must be finite.
The first goal in generation order returns immediately without inference.

Q-GBFS statistics additionally expose `visited_states`, `generated_transitions`,
and `evaluated_candidates`. `generated_transitions` counts all applied actions,
including duplicate successors, and stops at the first goal. The existing
`generated_states` field counts accepted search nodes, including the root and
priority improvements. `max_depth` is the maximum depth of an accepted node.
`action_values` aligns with the solution plan, or with `partial_plan` when
interrupted. The unevaluated final action of a solution has a `None` score
(`null` in C#); earlier actions retain their scores.
`should_stop(expanded_states)` is an optional callback after each search batch;
returning `True` produces `SearchStatus.STOPPED`. Timeouts and expansion limits
have their own statuses. An expansion limit finishes pending evaluation before
returning a partial plan. Cancellation avoids starting another evaluation;
a running evaluator cannot be preempted by a timeout.

In C#, `IHeuristic` overloads `Evaluate` for one state and a read-only state list.
`IQHeuristic` overloads it for one state/successor row and a read-only list of
those tuples. Inputs use named tuple aliases, and batch evaluation defaults to
the single-row overload. Call default implementations through the interface.
`SearchBuilder.WithQHeuristic(...).BuildQGbfs(...)` selects the new search.

## Beam search

`pymimir.beam(problem, heuristic, beam_size=4, max_depth=100)` and
`pymimir.qbeam(problem, q_heuristic, beam_size=4, max_depth=100)` share a native
beam loop. In C#, use `SearchBuilder.BuildBeam(...)` or `BuildQBeam(...)`.
Width one is greedy rollout. Every depth selects the best distinct successors
by incoming Q-value (higher by default, or lower with `maximize=False`) or by
successor heuristic value (lower). Scores are not accumulated along paths.
Equal scores retain the first transition in generation order.

Only states admitted to a beam become closed. A candidate discarded by the
width limit can be reached later. Q scoring receives a batch of complete parent
rows, including duplicate and closed successors. State heuristics evaluate each
distinct nonclosed successor once per layer, dispatching to the C# batch overload.
Built-in heuristics stay native; custom Python state heuristics receive the
unique candidates in `evaluate_batch`, whose default calls `evaluate` per state. State scores follow the standard heuristic contract:
nonnegative values, with positive infinity marking dead ends; NaN is rejected.
Q scores must all be finite.

The Python entry points check initial goals before action-generator setup or
heuristic binding. Both searches support alternate start states and goals, timeouts, expansion
limits, and `should_stop(expanded_states)` after an evaluated layer. Goals return
before evaluation with a `None` final action score. `max_depth` limits plan length
and returns `DEPTH_LIMIT_REACHED`; zero still checks the initial goal.
`DEAD_END` denotes a terminal layer without applicable actions or with only
infinite heuristic candidates. `EXHAUSTED` denotes successors that are all closed.
An expansion cap finishes scoring the collected rows. A fully expanded terminal
layer returns `DEAD_END` or `EXHAUSTED`, including at the exact expansion budget. Unsuccessful results retain
the best current beam path and its scores in `partial_plan` and `action_values`.

Beam statistics use `visited_states` for admitted states (including the root),
`generated_states` for admitted nodes, `generated_transitions` for applied actions,
and `evaluated_candidates` for Q entries or distinct state heuristic evaluations.
`max_depth` records the deepest admitted node. Generation stops at the first goal.

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
