<!-- EXAMPLE NEEDS TO BE UPDATED -->

# C++ Examples

The following examples show some common use cases for Mimir.
See also the [Python examples](EXAMPLES_PYTHON.md), the C++ implementation is often very similar.

## Parse and Solve Instance

The following code snippet parses and translates a domain `domain.pddl` and problem `problem.pddl`, creates a _search_context that contains a _lifted applicable action generator_, a _lifted axiom evaluator_, a _state repository_, and solves the problem using breadth-first search. The result of the search is stored in `status` and `plan`. If the instance is solvable, `status` will say so, and `plan` will contain a list of ground actions.

```cpp
#include  <mimir/mimir.hpp>

auto search_context_options = mimir::search::SearchContext::Options();
search_context_options.mode = mimir::search::SearchContext::SearchMode::LIFTED;
const auto search_context = mimir::search::SearchContext("domain.pddl", "problem.pddl", search_context_options);
const auto result = mimir::search::brfs::find_solution(search_context);
const auto status = result.status;
const auto plan = result.plan;
```
