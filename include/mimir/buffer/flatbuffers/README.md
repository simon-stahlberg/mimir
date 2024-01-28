# Generate Instructions

Add `dependencies/installs/bin` to `$PATH` before using.

```console
flatc --cpp bitset.fbs
flatc --cpp state-bitset-grounded.fbs
flatc --cpp state-bitset-lifted.fbs
flatc --cpp --gen-mutable search_node-cost.fbs
flatc --cpp action-default.fbs
```

Add flag `--gen-mutable` to generate mutable buffers
