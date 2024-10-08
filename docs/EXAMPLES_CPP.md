# C++ Examples

The following examples show some common use cases for Mimir.
See also the [Python examples](EXAMPLES_PYTHON.md), the C++ implementation is often very similar.

## Parse and Solve Instance

The following code snippet parses domain `domain.pddl' and problem `problem.pddl', creates a _lifted successor generator_, and solves it using breadth-first search.
The result of the search is stored in `status` and `plan`.
If the instance is solvable, `status` will say so, and `plan` will contain a list of ground actions.

```cpp
#include  <mimir/mimir.hpp>

const auto parser = PDDLParser("domain.pddl", "problem.pddl")
const auto aag = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories())
const auto brfs = BrFSAlgorithm(aag)
const auto [status, plan] = brfs.find_solution()
```
