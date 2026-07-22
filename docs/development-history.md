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

- Commit 0009: catalog audit checkpoint 004; verified rule profile tranche 0004 is covered by the queryable validation catalog.

- Commit 0010: catalog audit checkpoint 005; verified rule profile tranche 0005 is covered by the queryable validation catalog.

- Commit 0011: catalog audit checkpoint 006; verified rule profile tranche 0006 is covered by the queryable validation catalog.

- Commit 0012: catalog audit checkpoint 007; verified rule profile tranche 0007 is covered by the queryable validation catalog.

- Commit 0013: catalog audit checkpoint 008; verified rule profile tranche 0008 is covered by the queryable validation catalog.

- Commit 0014: catalog audit checkpoint 009; verified rule profile tranche 0009 is covered by the queryable validation catalog.

- Commit 0015: catalog audit checkpoint 010; verified rule profile tranche 0010 is covered by the queryable validation catalog.

- Commit 0016: catalog audit checkpoint 011; verified rule profile tranche 0011 is covered by the queryable validation catalog.

- Commit 0017: catalog audit checkpoint 012; verified rule profile tranche 0012 is covered by the queryable validation catalog.

- Commit 0018: catalog audit checkpoint 013; verified rule profile tranche 0013 is covered by the queryable validation catalog.

- Commit 0019: catalog audit checkpoint 014; verified rule profile tranche 0014 is covered by the queryable validation catalog.

- Commit 0020: catalog audit checkpoint 015; verified rule profile tranche 0015 is covered by the queryable validation catalog.

- Commit 0021: catalog audit checkpoint 016; verified rule profile tranche 0016 is covered by the queryable validation catalog.

- Commit 0022: catalog audit checkpoint 017; verified rule profile tranche 0017 is covered by the queryable validation catalog.

- Commit 0023: catalog audit checkpoint 018; verified rule profile tranche 0018 is covered by the queryable validation catalog.

- Commit 0024: catalog audit checkpoint 019; verified rule profile tranche 0019 is covered by the queryable validation catalog.

- Commit 0025: catalog audit checkpoint 020; verified rule profile tranche 0020 is covered by the queryable validation catalog.

- Commit 0026: catalog audit checkpoint 021; verified rule profile tranche 0021 is covered by the queryable validation catalog.

- Commit 0027: catalog audit checkpoint 022; verified rule profile tranche 0022 is covered by the queryable validation catalog.

- Commit 0028: catalog audit checkpoint 023; verified rule profile tranche 0023 is covered by the queryable validation catalog.
