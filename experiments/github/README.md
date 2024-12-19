
# Experiments

We compare Mimir against two state-of-the-art planning systems: 

1. [Fast Downward](https://github.com/aibasel/downward), which is optimized for grounded planning. The state representation in Fast Downward is a compact dense representation. More specifically it uses mutex analysis to translate propositional ground planning problems to finite-domain representation (FDR/SAS+). We turn off the additional preprocessing of computing irrelevant variables by setting the flag `--keep-unimportant-variables` to obtain equivalent search behavior.
2. [Powerlifted](https://github.com/abcorrea/powerlifted), which is optimized for lifted planning. The state representation in Powerlifted is a sparse vector representation.

The state representation in Mimir is a sparse vector representation with dynamic compression to smallest required unsigned integer type (uint8_t, uint16_t, uint32_t).

All experiments use a resource limit of 8 Gigabytes of memory and 30 minutes time.

## Benchmarks

1. Hard-To-Ground (HTG)

Hard to ground benchmarks result in often infeasibly large numbers of ground atoms and/or ground actions/axioms. We use a compilation of hard to ground benchmarks available on [GitHub](https://github.com/abcorrea/htg-domains).

2. International Planning Competition (IPC) - Optimal STRIPS

The optimal strips benchmarks from the IPC use a simple PDDL fragment, which includes `:strips`, `:action-costs`, `:typing`, `:negative-preconditions`, `:equality`. Additionally, these bencharks are often easy to ground, with only a small number of reached ground atoms, actions, and axioms. We use a compilation of the IPC benchmarks available on [Github](https://github.com/aibasel/downward-benchmarks).

3. International Planning Competition (IPC) - Optimal ADL

The optimal adl benchmarks from the IPC use a more expressive PDDL fragment, which additionally includes `:adl` and implies `:disjunctive-preconditions`, `:quantified-preconditions`, `:conditional-effects`. Similarly as in the STRIPS case, these benchmarks are often easy to ground and are available in the same [Github](https://github.com/aibasel/downward-benchmarks) repository.
