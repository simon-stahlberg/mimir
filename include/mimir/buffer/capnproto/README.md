# Generate Instructions

Add `dependencies/installs/bin` to `$PATH` before using.

```console
capnp compile -oc++ your_schema_file.capnp
```

Add flag `--gen-mutable` to generate mutable buffers
