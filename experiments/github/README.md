
# Experiments

We compare Mimir against two state-of-the-art planning systems [Fast Downward](https://github.com/aibasel/downward) and [Powerlifted](https://github.com/abcorrea/powerlifted) on three benchmark sets. The outline is as follows: first, we describe the objectives of our experimental evaluation. Second, we describe some important technical details of each planner and configurations. Third, we discuss the benchmark sets that were used. Last, we discuss some details on how to correctly interpret the data that we produced.

## Evaluation Objectives

Our experimental evaluation aims to get an understanding of the effectiveness of the internal data structures and algorithms used to efficiently search through the state space by iteratively expanding states in order to find the goal of a given planning problem. The performance depends primarily on the planners ability to compactly store states, search nodes, ground actions, and ground axioms, as well as efficiently generating the applicable actions for a given state.

## Common Setup

Our evaluation uses uninformed AStar search with a blind heuristic. While fairness could also be ensured by using the same heuristic across all planners, the choice of a blind heuristic simplifies the comparison by removing heuristic guidance entirely. This allows the focus to remain on the raw computational efficiency of the planners' internal data structures and algorithms.

To compare planners under resource constraints and test their ability to operate efficiently within both time and memory limitations, all experiments are limited to:

- `Memory limit`: 8 GB
- `Time limit`: 30 minutes

To evaluate the planners performances we use the following performance metrics:

- `Coverage`: the total number of solved problems
- `Time`: the geometric mean over the total time in milliseconds needed to solve all problems (considering only problems solved by all configurations)

## Planner Configurations

1. `Fast-Downward` is optimized for grounded planning. The state representation in Fast-Downward is a dense representation. More specifically it uses mutex analysis to translate propositional ground planning problems to finite-domain representation (FDR/SAS+). We turn off the additional preprocessing of computing irrelevant variables by setting the flag `--keep-unimportant-variables` to obtain equivalent search behavior.
2. `Powerlifted` is optimized for lifted planning. The state representation in Powerlifted is a sparse vector representation.
3. Mimir is optimized for lifted planning but additionally has a grounded mode. We denote its lifted configuration by `Mimir-lifted` and its grounded configuration by `Mimir-grounded` The state representation in Mimir is a sparse vector representation with dynamic compression to smallest required unsigned integer type. Mimir utilizes the high performance zero-copy serialization library (cista)[https://github.com/felixguendling/cista] to store states, search nodes, grounded actions and axioms as sequence of bytes in large segmented buffers.

## Benchmarks

1. `Hard-to-Ground` (HTG)

Hard to ground benchmarks result in often infeasibly large numbers of ground atoms and/or ground actions/axioms. We use a compilation of hard to ground benchmarks available on [GitHub](https://github.com/abcorrea/htg-domains).

2. `Optimal-STRIPS` from the international planning competition (IPC)

The optimal STRIPS benchmarks from the IPC use a simple PDDL fragment, which includes `:strips`, `:action-costs`, `:typing`, `:negative-preconditions`, `:equality`. Additionally, these bencharks are often easy to ground, with only a small number of reached ground atoms, actions, and axioms. We use a compilation of the IPC benchmarks available on [Github](https://github.com/aibasel/downward-benchmarks).

3. `Optimal-ADL` from the international planning competition (IPC)

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which additionally includes `:adl` and implies `:disjunctive-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly as in the STRIPS case, these benchmarks are often easy to ground and are available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.

## Comparison Notes and Clarifications

- Fast Downward and Mimir count the number of generated states differently. Mimir does currently not filter useless actions that when applied result in the same state again, e.g., `move(rooma, rooma)` in the Gripper domain.

- Powerlifted reports search time as total time and hence, we added the translator time on top to obtain the search time to obtain the total time.

## Experimental Results

It follows the performance metric scores `Coverage` and `Time` for all grounded configurations `Fast-Downward`, `Mimir-grounded` and lifted configurations `Powerlifted` and `Mimir-lifted` on the three benchmark sets `Hard-to-Ground`, `Optimal-STRIPS`, and `Optimal-ADL`.

1. Hard-to-Ground

| Item              |     Coverage |      Time |
| :---------------- | -----------: |     ----: |
| Fast-Downward     |          126 |      3132 |
| Mimir-grounded    |          106 |      4179 |
| Powerlifted       |          135 |    **344** |
| Mimir-lifted      |      **155** |       606 |

1. Optimal-STRIPS

| Item              |     Coverage |      Time |
| :---------------- | -----------: |     ----: |
| Fast-Downward     |      **752** |      1273 |
| Mimir-grounded    |          680 |   **973** |
| Powerlifted       |          539 |      6958 |
| Mimir-lifted      |          656 |      2959 |

3. Optimal-ADL

| Item              |     Coverage |      Time |
| :---------------- | -----------: |     ----: |
| Fast-Downward     |      **365** |      2085 |
| Mimir-grounded    |         328  |  **1469** |
| Powerlifted       |            x |         x |
| Mimir-lifted      |          293 |      5667 |

Observations:
- Fast-Downward does not have the overall best runtime performance, most likely due to its extensive preprocessing in Python. However, its preprocessing results in a very compact state representation, resulting in strong memory efficiency and overall highest coverage on the IPC STRIPS and ADL benchmarks.
- Powerlifted has the best runtime performance in Hard-to-Ground benchmarks but lower coverage compared to Mimir, most likely as a result of a worse state representation. Powerlifted does not support the PDDL extensions that were used in the IPC ADL benchmarks, therefore, we marked it as "x".
- Mimir has the highest coverage on Hard-To-Ground tasks and best runtime performance on the IPC STRIPS and ADL benchmarks.

## Conclusions

We conclude that Fast-Downward is the strongest planner in the grounded setting. Powerlifted is a strong planner in hard to ground tasks but performs significantly worse on easy to ground tasks in terms of coverage and runtime in the IPC STRIPS benchmarks. Mimirs performs strongly in hard to ground and easy to ground benchmarks, making it an overall strong competitor. Most importantly, its capability of running in lifted or grounded mode makes it a highly flexible system that can effectively adapt to benchmark requirements. Furthermore, Mimir supports a large fragment of PDDL, including ADL, which is not supported by Powerlifted.
