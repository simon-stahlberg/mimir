
# Experiments

## Resource Limits

- Memory: 8 Gigabytes
- Time: 30 Minutes

## State Representations

- Mimir: sparse vector representation with dynamic compression to smallest required unsigned integer type (uint8_t, uint16_t, uint32_t)
- Powerlifted: sparse vector representation (?)
- Fast Downward: dense finite-domain representation (FDR) obtained from mutex analysis during preprocessing