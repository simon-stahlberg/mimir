# PDDL Domain and Problem Parser Specification

This project implements a parser, semantic validator, and canonicalizer for PDDL domain and problem files. It will be implemented in C# using the **Superpower** library, separating the lexical tokenization phase from the parsing phase.

The system supports the following PDDL features: `:strips, :typing, :equality, :negative-preconditions, :disjunctive-preconditions, :numeric-fluents, :conditional-effects, :existential-preconditions, :universal-preconditions, :adl, :derived-predicates, :action-costs`.

## 1. Lexical Analysis and Tokenization
The system must use Superpower's `TokenizerBuilder` to convert the raw character stream into a token stream before parsing.
* **Case-Insensitivity:** PDDL is completely case-insensitive. The tokenizer/parser must handle this natively, and all AST lookups must use `StringComparer.OrdinalIgnoreCase`.
* **Comment Handling:** Immediately discard any text following a semicolon (`;`) until the end of the line.
* **LISP-Style S-Expressions:** Parentheses `(` and `)` are strict structural tokens.
* **Identifiers:** Allow hyphens and underscores (e.g., `move-to-location`). Hyphens are valid identifier characters and must not be confused with the subtraction operator unless contextually parsing a numeric fluent.
* **Variables vs. Constants:** The tokenizer must distinctly separate variables (prefixed with `?`, e.g., `?loc`) from constants (e.g., `loc1`).

## 2. Abstract Syntax Tree (AST) Design & API
The AST must act as both the compilation target for the parser and a programmatic API for developers.
* **Immutability:** The AST must be built using immutable C# `record` types to ensure safe canonicalization passes.
* **Fluent Builder Pattern:** Provide a fluent API for constructing instances programmatically (e.g., `DomainBuilder.Create("logistics").AddAction(a => a.WithName("move").WithParameters(...))`).
* **Round-Trip Serialization:** Every AST node must override `ToString()` to serialize itself back into perfectly formatted, valid PDDL text.
* **Expression Hierarchy:**
    * `ILogicalExpression`: For `And`, `Or`, `Not`, `Imply`, `Forall`, `Exists`, `PredicateCall`, and `Equality`.
    * `INumericExpression`: For `Add`, `Subtract`, `Multiply`, `Divide`, and fluent values.
    * `INumericEffect`:** For fluent state updates: `Assign`, `Increase`, `Decrease`, `ScaleUp`, `ScaleDown`.

## 3. Type System & Variable Scoping
Since the parser supports `:typing`, it needs to accurately represent hierarchical constraints.
* **The Root Type:** If `:typing` is enabled, any untyped object or parameter implicitly inherits from the built-in `object` root type.
* **Multiple Inheritance:** Handle multiple types assigned simultaneously (e.g., `?v1 ?v2 - vehicle`).
* **Scoping:** When programmatically building or parsing an action/axiom, the AST must ensure that variables used in preconditions and effects strictly reference the parameters defined in the schema signature.

## 4. Semantic Validation (Pre-Compilation)
Before canonicalization, a `Validator` pass must analyze the parsed AST for semantic correctness:
* **Arity Mismatch:** Calling a predicate with the wrong number of arguments (e.g., `(move ?x)` instead of `(move ?x ?y)`).
* **Undeclared Variables:** Using a variable in an effect or precondition that is not defined in the action's `:parameters` or bounded by a quantifier.
* **Type Conflicts:** Passing a parameter of type `location` to a predicate explicitly requiring a `vehicle`.
* **State Initialization:** Ensure numeric fluents are fully initialized in the Problem's `:init` block, and validate that no unsupported dynamic expressions exist in the initial state.

## 5. Canonicalization Engine
The canonicalizer transforms the rich AST into a simplified, canonical AST optimized for strict forward-chaining evaluation. It creates a new, transformed `DomainDefinition` and `ProblemDefinition`.

### 5.1 Action Canonicalization
All actions must be reduced to the following strict form:
* **Preconditions** must be a flat conjunction (`And`) of simple literals (positive or negative predicates).
* **Effects** must be a flat list of `ConditionalEffect` objects.

### 5.2 Transformation Rules
To achieve the canonical form, the compiler must apply these transformations:
* **Conditional Effect Normalization:** Nested `when` expressions are flattened (e.g., `(when (A) (when (B) (C)))` $\rightarrow$ `(when (and A B) (C))`). Unconditional effects are wrapped in a tautology (e.g., `[When(True, effect)]`). Multiple effects under one condition are distributed into separate conditional effects.
* **Disjunction & Implication Elimination:** `(imply P Q)` is converted to `(or (not P) Q)`. If an `or` statement exists in an action's precondition, the action is split into multiple independent actions representing the different logical branches.
* **Quantifier Relocation (Universal/Existential):** Quantifiers in preconditions must be stripped out and replaced with a call to a zero-arity Axiom (Derived Predicate).
  * `exists` in a precondition is replaced by an axiom that searches for a witness.
  * `forall` in a precondition is replaced by a `not` call to an axiom that searches for an existential counter-witness.
* **Action Costs Integration:** `:action-costs` are treated as syntactic sugar. They must be compiled into standard `INumericEffect` updates targeting a built-in `(total-cost)` fluent.

## 6. Testing and Validation
Comprehensive test coverage is required to verify parser integrity and compiler correctness:
* **Unit Tests:** For individual Superpower parsers (e.g., testing that `PredicateParser` correctly parses `(at ?x ?y)`).
* **Integration Tests:** Execute against the provided `Tests/Examples` folder containing standard PDDL benchmark domains (e.g., BlocksWorld, Logistics, Rovers).
* **Round-Trip Testing:** Verify that `Parse(pddl_string).ToString() == pddl_string` (ignoring whitespace and comments) to guarantee no data is lost during AST generation.
* **Canonicalization Tests:** Assert that output actions contain zero `Or`, `Imply`, `Forall`, or nested `When` nodes after compilation.
