# Python Examples

The following examples show some common use cases for Mimir.

## Parse and Solve Instance

The following code snippet parses domain `domain.pddl` and problem `problem.pddl`, creates a _lifted applicable action generator_, a _lifted axiom evaluator_, a _state repository_, and solves the problem using breadth-first search.
The result of the search is stored in `status` and `plan`.
If the instance is solvable, `result.status` will say so, and `result.plan.get_actions()` will return a list of ground actions.

```python
import pymimir as mm

parser = mm.PDDLParser("domain.pddl", "problem.pddl")
grounder = mm.Grounder(parser.get_problem(), parser.get_pddl_repositories())
successor_generator = mm.LiftedApplicableActionGenerator(grounder.get_action_grounder())
axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
state_repository = mm.StateRepository(axiom_evaluator)
initial_state = state_repository.get_or_create_initial_state()
result = mm.find_solution_brfs(successor_generator, state_repository, initial_state)
print(result.status)
print(str.join(', ', [action.to_string_for_plan(parser.get_pddl_repositories()) for action in result.plan.get_actions()]))
```

## Learning General Policies

The project [Relational Neural Networks](https://github.com/simon-stahlberg/relational-neural-network-python) is implemented with Mimir and learns general policies using supervised learning.
Please note that the project may be using an older version of Mimir and may not be compatible with the latest version.
