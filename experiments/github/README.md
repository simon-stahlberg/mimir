
# Experiments

We compare [Mimir](https://github.com/simon-stahlberg/mimir) against two state-of-the-art planning systems, [Fast Downward](https://github.com/aibasel/downward) and [Powerlifted](https://github.com/abcorrea/powerlifted), on three benchmark sets. The outline is as follows: First, we describe the objectives of our experimental evaluation. Second, we describe some important technical details of each planner and configurations that were used. Third, we discuss the benchmark sets that were used. Fourth, we will discuss some details on correctly interpreting the data we produced. Fifth, we present the gathered data and interpret it. Last, we conclude.

## Evaluation Objectives

Our experimental evaluation aims to understand the effectiveness of the internal data structures and algorithms used to efficiently search through the state space by iteratively expanding states to find the goal of a given planning problem. Performance depends primarily on the planner's ability to compactly store states, search nodes, ground actions, and ground axioms, and efficiently generate the applicable actions for a given state.

Our evaluation uses an uninformed AStar search with a blind heuristic. While fairness could also be ensured by using the same heuristic across all planners, the choice of a blind heuristic simplifies the comparison by removing heuristic guidance entirely. This allows the focus to remain on the raw computational efficiency of the planners' internal data structures and algorithms.

## Common Setup

We use the following resource constraint to test the planners' ability to operate efficiently within both time and memory limitations:

- `Memory limit`: 8 GB
- `Time limit`: 5 minutes and 30 minutes

We use the following performance metrics to compare the planners':

- `Coverage`: the total number of solved problems
- `Total time`: the geometric mean over the total time in milliseconds needed to solve all problems while considering only problems solved by all configurations.
- `Search time`: the geometric mean over the search time in milliseconds needed to solve all problems while considering only problems solved by all configurations.

## Planner Configurations

1. `Fast-Downward` is one of the most widely used planners that works exclusively on the grounded problem representation. The state representation in Fast-Downward is a dense finite-domain representation (FDR/SAS+) obtained through a preprocessing step called mutex analysis. Additionally, Fast-downward applied an additional pruning of unimportant variables to prune the search space. To obtain comparable search behavior regarding state expansions, we turn off the computation of irrelevant variables by setting the flag `--keep-unimportant-variables`.
2. `Powerlifted` is one of the most widely used planners that work on the lifted problem representation. The state representation in Powerlifted is a sparse vector representation. Powerlifted does not support `:negative-preconditions`.
3. Mimir is designed to work on grounded and lifted problem representations while being optimized for the lifted representation. We denote its lifted configuration as `Mimir-lifted` and its grounded configuration as `Mimir-grounded.` The state representation in Mimir is a sparse vector representation with dynamic compression to the smallest required unsigned integer type. 

## Benchmarks

1. `Hard-to-Ground` (HTG)

Hard-to-ground benchmarks result in often infeasibly large numbers of ground atoms and/or ground actions/axioms. We use a compilation of hard-to-ground benchmarks available on [GitHub](https://github.com/abcorrea/htg-domains).

2. `Optimal-STRIPS` from the international planning competition (IPC)

The optimal STRIPS benchmarks from the IPC use a simple PDDL fragment, which includes `:strips`, `:action-costs`, `:typing`, `:negative-preconditions`, `:equality`. Additionally, these benchmarks are often easy to ground, with only a few reached ground atoms, actions, and axioms. We use a compilation of the IPC benchmarks available on [Github](https://github.com/aibasel/downward-benchmarks).

3. `Optimal-ADL` from the international planning competition (IPC)

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which includes `:adl` and implies `:disjunctive-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly, these benchmarks are often easy to ground and available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.

## Comparison Notes and Clarifications

- Fast Downward and Mimir count the number of generated states differently. Mimir does not filter useless actions that, when applied, result in the same state again, e.g., `move(room, room)` in the Gripper domain.

- Powerlifted reports search time as total time, so we added the translator time to the search time to obtain the total time.

## Experimental Results

It follows the performance metric scores `Coverage`, `Total time`, and `Search time` for all grounded configurations `Fast-Downward`, `Mimir-grounded` and lifted configurations `Powerlifted` and `Mimir-lifted` on the three benchmark sets `Hard-to-Ground`, `Optimal-STRIPS`, and `Optimal-ADL`.

### Time limit 30 Minutes:

1. Hard-to-Ground

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |          126 |            3132 |              102 |
| Mimir-grounded    |          106 |            4179 |           **87** |
| Powerlifted       |          135 |         **344** |              217 |
| Mimir-lifted      |      **155** |             606 |              321 |

1. Optimal-STRIPS

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **752** |            1273 |          **261** |
| Mimir-grounded    |          680 |         **973** |              424 |
| Powerlifted       |          539 |            6958 |             6050 |
| Mimir-lifted      |          656 |            2959 |             2029 |

3. Optimal-ADL

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **365** |            2085 |          **325** |
| Mimir-grounded    |          328 |        **1469** |              646 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          293 |            5667 |             4367 |

Observations:
- Fast-Downward's preprocessing step results in the most compact state representation in easy-to-ground benchmarks, resulting in strong memory efficiency, search time, and overall highest coverage on the IPC STRIPS and ADL benchmarks. The preprocessing step causes a slight decline in total time compared to Mimir.
- Powerlifted has the best runtime performance in hard-to-ground benchmarks but lower coverage than Mimir because it does not not support the PDDL extensions used in ADL benchmarks (explicitly marked as x in the ADL table) and negative preconditions. In the unsupported domains in hard-to-ground and STRIPS, Mimir-lifted solves a total of 18, respectively 70, problems. Supporting these extensions in Powerlifted would result in potentially similar coverage scores.
- Fast-Downward's and Mimir's grounding step costs a comparable and significant amount of time in hard-to-ground benchmark.
- Overall, the planners complement each other nicely. Fast-Downward performs strongest in easy-to-ground benchmarks, while Mimir and Powerlifted perform strongest on hard-to-ground benchmarks.

### Time limit 5 Minutes:

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |          113 |            2552 |               67 |
| Mimir-grounded    |           98 |            3115 |           **49** |
| Powerlifted       |          129 |         **197** |              120 |
| Mimir-lifted      |      **136** |             342 |              176 |

1. Optimal-STRIPS

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **702** |             607 |           **88** |
| Mimir-grounded    |          654 |         **341** |              129 |
| Powerlifted       |          454 |            2113 |             1868 |
| Mimir-lifted      |          679 |             916 |              583 |

3. Optimal-ADL

| Item              |     Coverage | Total time [ms] | Search time [ms] |
| :---------------- | -----------: | --------------: | ---------------: |
| Fast-Downward     |      **318** |            1222 |          **132** |
| Mimir-grounded    |          301 |         **792** |              285 |
| Powerlifted       |            x |               x |                x |
| Mimir-lifted      |          252 |            2566 |             1840 |

The observations remain identical to the 30 minutes experiment.

## Conclusions

We conclude that Fast-Downward performs best in the grounded setting. Powerlifted is a strong planner in hard-to-ground tasks but performs significantly worse on easy-to-ground tasks regarding coverage, search time, and total time in the IPC STRIPS benchmarks. Mimir performs strongly in hard-to-ground and easy-to-ground benchmarks, making it an overall strong competitor. Most importantly, its capability of running in lifted or grounded mode makes it a highly flexible system that can easily adapt to benchmark requirements. Furthermore, Mimir's lifted mode supports an expressive fragment of PDDL, including ADL specifications.
