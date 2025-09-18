# Python Examples

The following examples show some common use cases for Mimir.

## Parse and Solve Instance

The following code snippet parses and translates a domain `domain.pddl` and problem `problem.pddl`, creates a _search_context that contains a _lifted applicable action generator_, a _lifted axiom evaluator_, a _state repository_, and solves the problem using breadth-first search. The result of the search is stored in `status` and `plan`. If the instance is solvable, `status` will say so, and `plan` will contain a list of ground actions.

```python
import pymimir.advanced.search as search

search_context = search.SearchContext.create("domain.pddl", "problem.pddl", search.SearchContextOptions(search.LiftedOptions()))
result = search.find_solution_brfs(search_context)
status = result.status
plan = result.plan
```

## Learning General Policies

The project [Relational Neural Networks](https://github.com/simon-stahlberg/relational-neural-network-python) is implemented with Mimir and learns general policies using supervised learning.
Please note that the project may be using an older version of Mimir and may not be compatible with the latest version.

## Expressivity Requirement Analysis

The project [Domain Expressivity Requirements Analysis](https://github.com/drexlerd/isomorphism-code) is implemented with Mimir and checks for failures to distinguish non-isomorphic states with the k-dimensional Weisfeiler-Leman algorithm (k-WL). The k-WL algorithm is an approximate graph isomorphism algorithm whose expressive power is equivalent to k+1 variable first-order logic with counting quantifiers.
