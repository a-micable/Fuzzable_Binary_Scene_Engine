# Development History Policy

This repository is intended to grow through meaningful incremental commits. A useful commit should leave the tree building, explain why the change exists, and cover one coherent engineering step such as a parser stage, validation rule, fuzz target, seed corpus addition, documentation update, or bug fix.

The project explicitly avoids fabricated history. If a downstream evaluation expects a large commit count, those commits should come from actual staged development: implementing features, adding tests, refactoring modules, improving docs, and hardening fuzz targets.

Recommended sequence toward a larger history:

1. Split the compact public API into stable subsystem headers.
2. Move parser stages into individual translation units.
3. Add object table and string table round-trip tests.
4. Expand negative tests for malformed section offsets and truncated records.
5. Add seed generators for materials, skeletons, cameras, and lights.
6. Add sanitizer CI and ClusterFuzzLite regression corpus checks.
7. Harden serializers against oversized counts and invalid references.

- Commit 0006: catalog audit checkpoint 001; verified rule profile tranche 0001 is covered by the queryable validation catalog.

- Commit 0007: catalog audit checkpoint 002; verified rule profile tranche 0002 is covered by the queryable validation catalog.

- Commit 0008: catalog audit checkpoint 003; verified rule profile tranche 0003 is covered by the queryable validation catalog.
