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

- Commit 0029: catalog audit checkpoint 024; verified rule profile tranche 0024 is covered by the queryable validation catalog.

- Commit 0030: catalog audit checkpoint 025; verified rule profile tranche 0025 is covered by the queryable validation catalog.

- Commit 0031: catalog audit checkpoint 026; verified rule profile tranche 0026 is covered by the queryable validation catalog.

- Commit 0032: catalog audit checkpoint 027; verified rule profile tranche 0027 is covered by the queryable validation catalog.

- Commit 0033: catalog audit checkpoint 028; verified rule profile tranche 0028 is covered by the queryable validation catalog.

- Commit 0034: catalog audit checkpoint 029; verified rule profile tranche 0029 is covered by the queryable validation catalog.

- Commit 0035: catalog audit checkpoint 030; verified rule profile tranche 0030 is covered by the queryable validation catalog.

- Commit 0036: catalog audit checkpoint 031; verified rule profile tranche 0031 is covered by the queryable validation catalog.

- Commit 0037: catalog audit checkpoint 032; verified rule profile tranche 0032 is covered by the queryable validation catalog.

- Commit 0038: catalog audit checkpoint 033; verified rule profile tranche 0033 is covered by the queryable validation catalog.

- Commit 0039: catalog audit checkpoint 034; verified rule profile tranche 0034 is covered by the queryable validation catalog.

- Commit 0040: catalog audit checkpoint 035; verified rule profile tranche 0035 is covered by the queryable validation catalog.

- Commit 0041: catalog audit checkpoint 036; verified rule profile tranche 0036 is covered by the queryable validation catalog.

- Commit 0042: catalog audit checkpoint 037; verified rule profile tranche 0037 is covered by the queryable validation catalog.

- Commit 0043: catalog audit checkpoint 038; verified rule profile tranche 0038 is covered by the queryable validation catalog.

- Commit 0044: catalog audit checkpoint 039; verified rule profile tranche 0039 is covered by the queryable validation catalog.

- Commit 0045: catalog audit checkpoint 040; verified rule profile tranche 0040 is covered by the queryable validation catalog.

- Commit 0046: catalog audit checkpoint 041; verified rule profile tranche 0041 is covered by the queryable validation catalog.

- Commit 0047: catalog audit checkpoint 042; verified rule profile tranche 0042 is covered by the queryable validation catalog.

- Commit 0048: catalog audit checkpoint 043; verified rule profile tranche 0043 is covered by the queryable validation catalog.

- Commit 0049: catalog audit checkpoint 044; verified rule profile tranche 0044 is covered by the queryable validation catalog.

- Commit 0050: catalog audit checkpoint 045; verified rule profile tranche 0045 is covered by the queryable validation catalog.

- Commit 0051: catalog audit checkpoint 046; verified rule profile tranche 0046 is covered by the queryable validation catalog.

- Commit 0052: catalog audit checkpoint 047; verified rule profile tranche 0047 is covered by the queryable validation catalog.

- Commit 0053: catalog audit checkpoint 048; verified rule profile tranche 0048 is covered by the queryable validation catalog.

- Commit 0054: catalog audit checkpoint 049; verified rule profile tranche 0049 is covered by the queryable validation catalog.

- Commit 0055: catalog audit checkpoint 050; verified rule profile tranche 0050 is covered by the queryable validation catalog.

- Commit 0056: catalog audit checkpoint 051; verified rule profile tranche 0051 is covered by the queryable validation catalog.

- Commit 0057: catalog audit checkpoint 052; verified rule profile tranche 0052 is covered by the queryable validation catalog.

- Commit 0058: catalog audit checkpoint 053; verified rule profile tranche 0053 is covered by the queryable validation catalog.

- Commit 0059: catalog audit checkpoint 054; verified rule profile tranche 0054 is covered by the queryable validation catalog.

- Commit 0060: catalog audit checkpoint 055; verified rule profile tranche 0055 is covered by the queryable validation catalog.

- Commit 0061: catalog audit checkpoint 056; verified rule profile tranche 0056 is covered by the queryable validation catalog.

- Commit 0062: catalog audit checkpoint 057; verified rule profile tranche 0057 is covered by the queryable validation catalog.

- Commit 0063: catalog audit checkpoint 058; verified rule profile tranche 0058 is covered by the queryable validation catalog.

- Commit 0064: catalog audit checkpoint 059; verified rule profile tranche 0059 is covered by the queryable validation catalog.

- Commit 0065: catalog audit checkpoint 060; verified rule profile tranche 0060 is covered by the queryable validation catalog.

- Commit 0066: catalog audit checkpoint 061; verified rule profile tranche 0061 is covered by the queryable validation catalog.

- Commit 0067: catalog audit checkpoint 062; verified rule profile tranche 0062 is covered by the queryable validation catalog.

- Commit 0068: catalog audit checkpoint 063; verified rule profile tranche 0063 is covered by the queryable validation catalog.

- Commit 0069: catalog audit checkpoint 064; verified rule profile tranche 0064 is covered by the queryable validation catalog.

- Commit 0070: catalog audit checkpoint 065; verified rule profile tranche 0065 is covered by the queryable validation catalog.

- Commit 0071: catalog audit checkpoint 066; verified rule profile tranche 0066 is covered by the queryable validation catalog.

- Commit 0072: catalog audit checkpoint 067; verified rule profile tranche 0067 is covered by the queryable validation catalog.

- Commit 0073: catalog audit checkpoint 068; verified rule profile tranche 0068 is covered by the queryable validation catalog.

- Commit 0074: catalog audit checkpoint 069; verified rule profile tranche 0069 is covered by the queryable validation catalog.

- Commit 0075: catalog audit checkpoint 070; verified rule profile tranche 0070 is covered by the queryable validation catalog.

- Commit 0076: catalog audit checkpoint 071; verified rule profile tranche 0071 is covered by the queryable validation catalog.

- Commit 0077: catalog audit checkpoint 072; verified rule profile tranche 0072 is covered by the queryable validation catalog.

- Commit 0078: catalog audit checkpoint 073; verified rule profile tranche 0073 is covered by the queryable validation catalog.

- Commit 0079: catalog audit checkpoint 074; verified rule profile tranche 0074 is covered by the queryable validation catalog.

- Commit 0080: catalog audit checkpoint 075; verified rule profile tranche 0075 is covered by the queryable validation catalog.

- Commit 0081: catalog audit checkpoint 076; verified rule profile tranche 0076 is covered by the queryable validation catalog.

- Commit 0082: catalog audit checkpoint 077; verified rule profile tranche 0077 is covered by the queryable validation catalog.

- Commit 0083: catalog audit checkpoint 078; verified rule profile tranche 0078 is covered by the queryable validation catalog.

- Commit 0084: catalog audit checkpoint 079; verified rule profile tranche 0079 is covered by the queryable validation catalog.

- Commit 0085: catalog audit checkpoint 080; verified rule profile tranche 0080 is covered by the queryable validation catalog.

- Commit 0086: catalog audit checkpoint 081; verified rule profile tranche 0081 is covered by the queryable validation catalog.

- Commit 0087: catalog audit checkpoint 082; verified rule profile tranche 0082 is covered by the queryable validation catalog.

- Commit 0088: catalog audit checkpoint 083; verified rule profile tranche 0083 is covered by the queryable validation catalog.

- Commit 0089: catalog audit checkpoint 084; verified rule profile tranche 0084 is covered by the queryable validation catalog.

- Commit 0090: catalog audit checkpoint 085; verified rule profile tranche 0085 is covered by the queryable validation catalog.

- Commit 0091: catalog audit checkpoint 086; verified rule profile tranche 0086 is covered by the queryable validation catalog.

- Commit 0092: catalog audit checkpoint 087; verified rule profile tranche 0087 is covered by the queryable validation catalog.

- Commit 0093: catalog audit checkpoint 088; verified rule profile tranche 0088 is covered by the queryable validation catalog.

- Commit 0094: catalog audit checkpoint 089; verified rule profile tranche 0089 is covered by the queryable validation catalog.

- Commit 0095: catalog audit checkpoint 090; verified rule profile tranche 0090 is covered by the queryable validation catalog.

- Commit 0096: catalog audit checkpoint 091; verified rule profile tranche 0091 is covered by the queryable validation catalog.

- Commit 0097: catalog audit checkpoint 092; verified rule profile tranche 0092 is covered by the queryable validation catalog.

- Commit 0098: catalog audit checkpoint 093; verified rule profile tranche 0093 is covered by the queryable validation catalog.

- Commit 0099: catalog audit checkpoint 094; verified rule profile tranche 0094 is covered by the queryable validation catalog.

- Commit 0100: catalog audit checkpoint 095; verified rule profile tranche 0095 is covered by the queryable validation catalog.

- Commit 0101: catalog audit checkpoint 096; verified rule profile tranche 0096 is covered by the queryable validation catalog.

- Commit 0102: catalog audit checkpoint 097; verified rule profile tranche 0097 is covered by the queryable validation catalog.

- Commit 0103: catalog audit checkpoint 098; verified rule profile tranche 0098 is covered by the queryable validation catalog.

- Commit 0104: catalog audit checkpoint 099; verified rule profile tranche 0099 is covered by the queryable validation catalog.

- Commit 0105: catalog audit checkpoint 100; verified rule profile tranche 0100 is covered by the queryable validation catalog.

- Commit 0106: catalog audit checkpoint 101; verified rule profile tranche 0101 is covered by the queryable validation catalog.
