
# Experiments

We compare [Mimir](https://github.com/simon-stahlberg/mimir) against two state-of-the-art planning systems, [Fast Downward](https://github.com/aibasel/downward) and [Powerlifted](https://github.com/abcorrea/powerlifted), on four benchmark sets. The outline is as follows: First, we describe the objectives of our experimental evaluation. Second, we describe some common setup, including resource limits and performance metrics. Third, we describe some important technical details of each planner and configurations that were used. Fourth, we discuss the benchmark sets that were used. Last, we present the gathered data.

## 1. Evaluation Objectives

Our experimental evaluation aims to understand the effectiveness of the internal data structures and algorithms used to efficiently search through the state space by iteratively expanding states to find the goal of a given planning problem. Performance depends primarily on the planner's ability to compactly store states, search nodes, ground actions, and ground axioms, and efficiently generate the applicable actions for a given state.

Our evaluation uses an uninformed AStar search with a blind heuristic. While fairness could also be ensured by using the same heuristic across all planners, the choice of a blind heuristic simplifies the comparison by removing heuristic guidance entirely. This allows the focus to remain on the raw computational efficiency of the planners' internal data structures and algorithms.

## 2. Common Setup

We use the following resource constraint to test the planners' ability to operate efficiently within both time and memory limitations:

- `Memory limit`: 8 GB
- `Time limit`: 5 minutes and 30 minutes

We use the following performance metrics to compare the planners':

- `Coverage`: the total number of solved problems
- `Total time`: the geometric mean over the total time in milliseconds needed to solve all problems while considering only problems solved by all configurations.
- `Search time`: the geometric mean over the search time in milliseconds needed to solve all problems while considering only problems solved by all configurations.

## 3. Planner Configurations

1. `Fast-Downward` is one of the most widely used planners that works exclusively on the grounded problem representation. The state representation in Fast-Downward is a dense finite-domain representation (FDR/SAS+) obtained through a preprocessing step called mutex analysis. Additionally, Fast-downward applied an additional pruning of unimportant variables to prune the search space. To obtain comparable search behavior regarding state expansions, we turn off the computation of irrelevant variables by setting the flag `--keep-unimportant-variables`.
2. `Powerlifted` is one of the most widely used planners that work on the lifted problem representation. The state representation in Powerlifted is a sparse vector representation. Powerlifted does not support `:negative-preconditions`, `:conditional-effects`, `existential-quantifiers`, `universal-quantifiers`. We will discuss its impact on the experimental results for a fair comparison.
3. Mimir is designed to work on grounded and lifted problem representations while being optimized for the lifted representation. We denote its lifted configuration as `Mimir-lifted` and its grounded configuration as `Mimir-grounded.` The state representation in Mimir is a sparse vector representation with dynamic compression to the smallest required bitwidth.

## 4. Benchmarks

1. `Optimal-STRIPS` from the international planning competition (IPC)

The optimal STRIPS benchmarks from the IPC use a simple PDDL fragment, which includes `:strips`, `:action-costs`, `:typing`, `:negative-preconditions`, `:equality`. We exclude domains that use negative preconditions because Powerlifted does not support it. Additionally, these benchmarks are often easy to ground, with only a few reached ground atoms, actions, and axioms. We use a compilation of the IPC benchmarks available on [Github](https://github.com/aibasel/downward-benchmarks).

2. `Hard-to-Ground` (HTG)

Hard-to-ground benchmarks often result in infeasibly high numbers of ground atoms and/or ground actions/axioms. We use a compilation of hard-to-ground benchmarks available on [GitHub](https://github.com/abcorrea/htg-domains). We exclude domains beyond the PDDL language features specified for the STRIPS benchmark set. 

3. `Optimal-ADL` from the International Planning Competition (IPC)

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which includes `:adl` and implies `:disjunctive-preconditions`, `:existential-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly, these benchmarks are often easy to ground and available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.

4. `Numeric` from the International Planning Competition (IPC)

The numeric benchmarks from the IPC use `numeric-fluents` and `:adl`. Similarly, these benchmarks are often easy to ground and available in the [Github](https://github.com/ipc2023-numeric/ipc2023-dataset) repository.

## 5. Comparison Notes and Clarifications

- Fast Downward and Mimir count the number of generated states differently. Mimir does not filter useless actions that, when applied, result in the same state again, e.g., `move(room, room)` in the Gripper domain.

- Powerlifted reports search time as total time, so we added the translator time to the search time to obtain the total time.

## 6. Experimental Results

It follows the performance metric scores `Coverage`, `Total time`, and `Search time` for all grounded configurations `Fast-Downward`, `Mimir-grounded` and lifted configurations `Powerlifted` and `Mimir-lifted` on the four benchmark sets `Hard-to-Ground`, `Optimal-STRIPS`, `Optimal-ADL`, and `Numeric`.

### Time limit 30 Minutes:

1. Optimal-STRIPS

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **659** |            1287 |          **265** |
| Mimir-grounded    |          606 |         **821** |              353 |
| Powerlifted       |          539 |            7008 |             6094 |
| Mimir-lifted      |          601 |            2618 |             1607 |

2. Hard-to-Ground

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |          108 |            4895 |              152 |
| Mimir-grounded    |          102 |            4770 |          **111** |
| Powerlifted       |          135 |             582 |              359 |
| Mimir-lifted      |      **143** |         **556** |              267 |

3. Optimal-ADL

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **365** |            2321 |          **384** |
| Mimir-grounded    |          332 |        **1314** |              578 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          299 |            5082 |             3641 |

4. Numeric

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |            x |               x |                x |
| Mimir-grounded    |           44 |        **1222** |          **686** |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |       **46** |            1806 |             1144 |

### Time limit 5 Minutes:

1. Optimal-STRIPS

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **617** |             607 |           **88** |
| Mimir-grounded    |          597 |         **312** |              110 |
| Powerlifted       |          454 |            2113 |             1868 |
| Mimir-lifted      |          530 |             841 |              502 |

2. Hard-to-Ground

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |           97 |            3142 |               71 |
| Mimir-grounded    |           91 |            2567 |           **53** |
| Powerlifted       |          129 |         **289** |              182 |
| Mimir-lifted      |      **132** |             381 |              145 |

3. Optimal-ADL

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **318** |            1401 |          **165** |
| Mimir-grounded    |          308 |         **637** |              257 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          261 |            2436 |             1664 |

4. Numeric

| Configuration     |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |            x |               x |                x |
| Mimir-grounded    |       **41** |         **535** |          **291** |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |       **41** |             806 |              509 |
