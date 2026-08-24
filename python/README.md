# Mimir Python API

Mimir 0.4 provides a typed, semantic API for loading PDDL and prototyping
search algorithms.

```python
from mimir import LiftedFFHeuristic, Problem, SearchStatus, astar

problem = Problem.from_files("domain.pddl", "problem.pddl")
heuristic = LiftedFFHeuristic(problem)
result = astar(problem, heuristic, timeout_seconds=30)

if result.solution is not None:
    for action in result.solution.plan:
        print(action, action.cost)
```

Use `problem.fact("at", "robot", "room")`, `problem.action(...)`, and
`problem.state(...)` to construct values without editing PDDL files. Search
callbacks receive immutable `Transition` records. The `mimir.advanced`
module is an unstable implementation detail.

Run the complete example with:

```console
python python/examples/quickstart.py domain.pddl problem.pddl
```
