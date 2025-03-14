<!-- EXAMPLE NEEDS TO BE UPDATED -->

# C++ Examples

The following examples show some common use cases for Mimir.
See also the [Python examples](EXAMPLES_PYTHON.md), the C++ implementation is often very similar.

## Parse and Solve Instance

The following code snippet parses domain `domain.pddl` and problem `problem.pddl`, creates a _lifted applicable action generator_, a _lifted axiom evaluator_, a _state repository_, and solves the problem using breadth-first search.
The result of the search is stored in `status` and `plan`.
If the instance is solvable, `status` will say so, and `plan` will contain a list of ground actions.

```cpp
#include  <mimir/mimir.hpp>

const auto parser = PDDLParser("domain.pddl", "problem.pddl");
const auto applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories());
const auto axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(parser.get_problem(), parser.get_pddl_repositories()));
const auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
const auto brfs = BrFSAlgorithm(state_repository);
const auto result = brfs.find_solution();
const auto status = result.status
const auto plan = result.plan
```
