# AI Assistant Guidelines

These guidelines are strictly enforced to prevent overengineering, bloated architecture, and unnecessary abstractions. Optimize for C# simplicity, human readability, and explicit failure.

## 1. Fail Fast (No Silent Fallbacks)
- **Never write fallback code that silently suppresses bugs.**
- Never use empty `catch (Exception)` blocks. Do not catch exceptions unless you are explicitly handling them or logging them at a boundary.
- If an unexpected state occurs, throw an explicit, standard exception (e.g., `InvalidOperationException`, `ArgumentNullException`). Do not return `null` or `default` values to sweep errors under the rug.

## 2. No Future-Proofing (YAGNI)
- **Never write backward compatibility code unless explicitly asked.** Assume we are moving forward.
- Do not build generic type hierarchies, abstract classes, or extensible patterns for use cases that do not exist yet. Build exactly what is needed today.
- **Do not blindly extract interfaces.** Do not create an `ISomething` interface for a class unless there are actually multiple implementations, or it is strictly required for mocking in existing tests.

## 3. The Rule of Three (No Premature Abstraction)
- Do not extract code into reusable methods, classes, or components until it is duplicated in at least three distinct places.
- Accept minor duplication if the abstraction would make the code harder to read or require heavy configuration.

## 4. Collocation (File Placement)
- **Defy the traditional "one type per file" rule when appropriate.** Place related `record`s, `enum`s, helper `class`es, and DTOs in the exact same `.cs` file where they are primarily used.
- Do not create separate folders or files for simple types unless the code is genuinely required by multiple distinct domains.
- Only split a `.cs` file if it becomes difficult to navigate (e.g., > 400 lines).

## 5. Complexity & Readability
- **Human readability is the highest priority.**
- Prefer early returns (Guard Clauses). Avoid `else` blocks; return early from the `if` block instead.
- **LINQ vs. Loops:** Use LINQ when it makes intent clear. However, avoid massive, chained LINQ queries. If a LINQ query spans more than 5 lines or contains complex nested logic, break it down or use a `foreach` loop.
- **Pattern Matching:** Utilize modern C# pattern matching (e.g., `switch` expressions) for brevity, but do not nest them so deeply that they become impossible to read.

## 6. Dependencies & Deletion
- **Zero Dependency Bias:** Prefer the .NET Base Class Library (BCL) over introducing new NuGet packages. Only suggest a third-party dependency if the native solution is highly complex.
- **Delete Dead Code Aggressively:** If a change makes old code, types, or variables obsolete, delete them completely. We have Git for reference.

## 7. Type System Discipline
- **Keep Types Simple:** Prefer simple concrete classes and `record` types over complex generics and Reflection.
- Never use `dynamic` silently. Strongly type everything. If you must bypass the static type checker using `dynamic` or heavy `Reflection`, add a `// Justification: ...` comment.
- Prefer explicit types over `var` when the right-hand side of the assignment does not clearly indicate the type (e.g., `var user = GetUser()` is bad; `User user = GetUser()` or `var user = new User()` is good).

## 8. Workflow & Planning
- **Plan Before Coding:** For any feature or refactor requiring more than 2 file changes or ~50 lines of code, outline your plan in a brief bulleted list first. Wait for my approval before writing the implementation.
- If you are unsure about the architectural fit of a solution, ask a single, clarifying question rather than guessing and building the wrong thing.

## 9. Communication & Modifications
- Do not add XML documentation (`/// <summary>`) or standard comments explaining *what* code does (e.g., `// Fetches user from DB`). Only comment *why* a specific, non-obvious decision was made.
- Only modify the code necessary to complete the user's request. Do not aggressively refactor surrounding code unless explicitly instructed.

## 10. Stack-Specific Rules (Customize for your specific .NET stack)
- **Async/Await:** Always use `await`. Never use `async void` (except for UI event handlers). Never use `.Result` or `.Wait()`, which can cause deadlocks.
- **Data Access (e.g., EF Core):** Beware of N+1 queries. Prefer projections (`.Select()`) for read-only data rather than tracking full entities.
- **Dependency Injection:** Register services with the minimum required lifetime. Default to `Transient` or `Scoped`; use `Singleton` sparingly and ensure thread safety.

## 11. Questions
- When asking the operator questions during planning, do not proceed based on a default answer if the operator hasn't responded. Treat unanswered questions as blocking decisions. Wait for explicit responses.
- Set the option `request_user_input` and `autoResolutionMs` to disable timeout so that the operator can answer when available.
