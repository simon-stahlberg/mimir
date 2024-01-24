# Search Nodes

- A search node tag represents a specific implementation for a search node
and depends on the planning mode tag, state tag, and action tag.

- For each tag, a single implementation suffices because we can use and provide and general interface.

- Alternative state implementations can be added by deriving from `SearchNodeBase`.
