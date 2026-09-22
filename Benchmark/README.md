# Benchmark layout

- `strips/`: 57 STRIPS-style domains, including typing, equality, negative
  preconditions, and action costs.
- `adl/`: 21 domains using conditional effects, quantified or disjunctive
  conditions, or derived predicates.
- `numeric/`: 10 numeric domains with C++ breadth-first baselines recorded in
  `Tests/Mimir.Search.Tests/NumericBaselineTests.cs`.

Classification follows the constructs in the checked-in PDDL, rather than domain
names or unused requirement declarations. In particular, `spider-opt18-strips`
uses conditional effects and is under `adl/`.

The lifted benchmark runner discovers domains under `strips/` and `adl/`.
`NumericBaselineTests` uses `numeric/`. Parser round-trip tests recursively
discover PDDL files in all three categories.
