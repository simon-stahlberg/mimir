# Python Examples

The following examples show some common use cases for Mimir.

## Parse and Solve Instance

The following code snippet parses domain `domain.pddl' and problem `problem.pddl', creates a _lifted successor generator_, and solves it using breadth-first search.
The result of the search is stored in `status` and `plan`.
If the instance is solvable, `status` will say so, and `plan` will contain a list of ground actions.

```python
import pymimir as mm

parser = mm.PDDLParser("domain.pddl", "problem.pddl")
applicable_action_generator = mm.LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_repositories())
brfs = mm.BrFSAlgorithm(applicable_action_generator)
status, plan = brfs.find_solution()
```

## Learning General Policies

The project [Relational Neural Networks](https://github.com/simon-stahlberg/relational-neural-network-python) is implemented with Mimir and learns general policies using supervised learning.
Please note that the project may be using an older version of Mimir and may not be compatible with the latest version.
