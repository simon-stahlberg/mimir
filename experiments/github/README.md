
# Experiments

We compare [Mimir](https://github.com/simon-stahlberg/mimir) against two state-of-the-art planning systems, [Fast Downward](https://github.com/aibasel/downward) and [Powerlifted](https://github.com/abcorrea/powerlifted), on three benchmark sets. The outline is as follows: First, we describe the objectives of our experimental evaluation. Second, we describe some common setup, including resource limits and performance metrics. Third, we describe some important technical details of each planner and configurations that were used. Fourth, we discuss the benchmark sets that were used. Fifth, we will discuss some details on correctly interpreting the data we produced. Sixth, we present the gathered data and interpret it. Last, we conclude.

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

3. `Optimal-ADL` from the international planning competition (IPC)

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which includes `:adl` and implies `:disjunctive-preconditions`, `:existential-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly, these benchmarks are often easy to ground and available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.

## 5. Comparison Notes and Clarifications

- Fast Downward and Mimir count the number of generated states differently. Mimir does not filter useless actions that, when applied, result in the same state again, e.g., `move(room, room)` in the Gripper domain.

- Powerlifted reports search time as total time, so we added the translator time to the search time to obtain the total time.

## 6. Experimental Results

It follows the performance metric scores `Coverage`, `Total time`, and `Search time` for all grounded configurations `Fast-Downward`, `Mimir-grounded` and lifted configurations `Powerlifted` and `Mimir-lifted` on the three benchmark sets `Hard-to-Ground`, `Optimal-STRIPS`, and `Optimal-ADL`.

### Time limit 30 Minutes:

1. Optimal-STRIPS

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **659** |            1273 |          **261** |
| Mimir-grounded    |          611 |         **756** |              313 |
| Powerlifted       |          539 |            6958 |             6050 |
| Mimir-lifted      |          598 |            2574 |             1692 |

2. Hard-to-Ground

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |          108 |            3132 |              102 |
| Mimir-grounded    |           88 |            3270 |           **61** |
| Powerlifted       |          135 |         **344** |              217 |
| Mimir-lifted      |      **137** |             522 |              270 |

3. Optimal-ADL

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **365** |            2394 |          **399** |
| Mimir-grounded    |          336 |        **1334** |              599 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          299 |            5722 |             4544 |

Observations:
- Fast-Downward's preprocessing step results in the most compact state representation on easy-to-ground benchmarks, resulting in strong memory efficiency, search time, and overall highest coverage on the IPC STRIPS and ADL benchmarks. The preprocessing step is costly, resulting in a higher total time than Mimir-grounded. Fast Downward and Mimir-grounded achieve comparable search time scores.
- Powerlifted has the best runtime score on hard-to-ground benchmarks and comparable coverage with Mimir-lifted. However, Powerlifted achieves significantly worse scores in all three performance metrics than Mimir-lifted in easy-to-ground benchmarks.
- Fast-Downward's and Mimir's grounding step costs a comparable and significant amount of time on hard-to-ground benchmarks. Mimir's costs are even higher because the tree structure used to retrieve applicable actions is based on a less compact propositional state representation than Fast Downward's, which is based on a compact finite-domain state representation (FDR).

### Time limit 5 Minutes:

1. Optimal-STRIPS

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **617** |             607 |           **88** |
| Mimir-grounded    |          592 |         **303** |              99 |
| Powerlifted       |          454 |            2113 |             1868 |
| Mimir-lifted      |          525 |             853 |              527 |

2. Hard-to-Ground

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |          97 |            2552 |               67 |
| Mimir-grounded    |          87 |            2458 |           **37** |
| Powerlifted       |     **129** |         **197** |              120 |
| Mimir-lifted      |         123 |             311 |              165 |

3. Optimal-ADL

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **318** |            1234 |          **137** |
| Mimir-grounded    |          312 |         **607** |              208 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          255 |            2340 |             1619 |

The observations remain almost identical to the 30 minutes experiment. The gaps between grounded planners planners starts to diminish, indicating that memory becomes less problematic. For lifted planners on the STRIPS benchmarks, the gap remains significant.

## 7. Conclusions

On easy-to-ground benchmarks, Fast-Downward has the most efficient state representation to generate the largest amount of states. Mimir-grounded has a comparable runtime score. Powerlifted scores significantly lower than Mimir-lifted, which seems out of reach, even when extending its PDDL support.

On hard-to-ground benchmarks, Powerlifted is a strong planner with the lowest total time. Mimir-lifted has the highest coverage score and comparable runtime performance.

Our experiments demonstrate that a single library can efficiently combine lifted and grounded planning with expressive extensions such as conditional effects, existential preconditions, and universal preconditions.

## 8. Future Work

In the future, we plan on further extending the supported PDDL fragments towards probabilistic, nondeterministic, and numeric planning.
