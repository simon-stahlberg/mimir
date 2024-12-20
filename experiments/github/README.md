
# Experiments

We compare Mimir against two state-of-the-art planning systems [Fast Downward](https://github.com/aibasel/downward) and [Powerlifted](https://github.com/abcorrea/powerlifted) on three benchmark sets. The outline is as follows: first, we describe the objectives of our experimental evaluation. Second, we describe some important technical details of each planner and configurations. Third, we discuss the benchmark sets that were used. Last, we discuss some details on how to correctly interpret the data that we produced.

## Evaluaton Objectives

Our experimental evaluation aims to get an understanding of the effectiveness of the internal data structures and algorithms used to efficiently search through the state space by iteratively expanding states in order to find the goal of a given planning problem. The performance depends primarily on the planners ability to compactly store states, search nodes, ground actions, and ground axioms, as well as efficiently generating the applicable actions for a given state. 

## Common Setup

Our evaluation uses uninformed AStar search with a blind heuristic. While fairness could also be ensured by using the same heuristic across all planners, the choice of a blind heuristic simplifies the comparison by removing heuristic guidance entirely. This allows the focus to remain on the raw computational efficiency of the planners' internal data structures and algorithms.

To compare planners under resource constraints and test their ability to operate efficiently within both time and memory limitations, all experiments are limited to:

- 8 GB memory
- 30 minutes time

## Planners

1. Fast Downward is optimized for grounded planning. The state representation in Fast Downward is a dense representation. More specifically it uses mutex analysis to translate propositional ground planning problems to finite-domain representation (FDR/SAS+). We turn off the additional preprocessing of computing irrelevant variables by setting the flag `--keep-unimportant-variables` to obtain equivalent search behavior.
2. Powerlifted is optimized for lifted planning. The state representation in Powerlifted is a sparse vector representation.
3. Mimir is optimized for lifted planning but additionally has a grounded mode. The state representation in Mimir is a sparse vector representation with dynamic compression to smallest required unsigned integer type. Mimir utilizes the high performance zero-copy serialization library (cista)[https://github.com/felixguendling/cista] to store states, search nodes, grounded actions and axioms as sequence of bytes in large segmented buffers.

## Benchmarks

1. Hard-to-Ground (HTG)

Hard to ground benchmarks result in often infeasibly large numbers of ground atoms and/or ground actions/axioms. We use a compilation of hard to ground benchmarks available on [GitHub](https://github.com/abcorrea/htg-domains).

2. International Planning Competition (IPC) - Optimal STRIPS

The optimal STRIPS benchmarks from the IPC use a simple PDDL fragment, which includes `:strips`, `:action-costs`, `:typing`, `:negative-preconditions`, `:equality`. Additionally, these bencharks are often easy to ground, with only a small number of reached ground atoms, actions, and axioms. We use a compilation of the IPC benchmarks available on [Github](https://github.com/aibasel/downward-benchmarks).

3. International Planning Competition (IPC) - Optimal ADL

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which additionally includes `:adl` and implies `:disjunctive-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly as in the STRIPS case, these benchmarks are often easy to ground and are available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.

## Comparison Notes and Clarifications

- Fast Downward and Mimir count the number of generated states differently. Mimir does currently not filter useless actions that when applied result in the same state again, e.g., `move(rooma, rooma)` in the Gripper domain.

- Powerlifted reports search time as total time and hence, we added the translator time on top to obtain the search time to obtain the total time.

## Conclusions


