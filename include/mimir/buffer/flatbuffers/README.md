# Generate Instructions

Add `dependencies/installs/bin` to `$PATH` before using.

```console
flatc --cpp bitset.fbs
flatc --cpp state-bitset.fbs
flatc --cpp --gen-mutable search_node-cost.fbs
flatc --cpp action-bitset.fbs
```

Add flag `--gen-mutable` to generate mutable buffers
