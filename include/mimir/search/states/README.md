# States

- A state tag represents a specific implementation for a state
and depends on the planning mode tag.

- For each state tag, we need two implementations:
one for the grounded and one for the lifted planning mode tag
because we want to provide different interfaces.

- Alternative state implementations can be added by deriving from `StateTagBase`.
