hack-illinois-kvstore
=====================

## Tentative Plan:
1. Basic single-machine string-to-string key-value store, backed by a single storage type
..* either only disk or only memory — not sure what’s better to start with
2. Integrate second storage type — utilize memory as a cache for the disk data
3. Different data types (eg, sets, lists) working in disk and memory
4. Clustering support
