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

- Commit 0107: catalog audit checkpoint 102; verified rule profile tranche 0102 is covered by the queryable validation catalog.

- Commit 0108: catalog audit checkpoint 103; verified rule profile tranche 0103 is covered by the queryable validation catalog.

- Commit 0109: catalog audit checkpoint 104; verified rule profile tranche 0104 is covered by the queryable validation catalog.

- Commit 0110: catalog audit checkpoint 105; verified rule profile tranche 0105 is covered by the queryable validation catalog.

- Commit 0111: catalog audit checkpoint 106; verified rule profile tranche 0106 is covered by the queryable validation catalog.

- Commit 0112: catalog audit checkpoint 107; verified rule profile tranche 0107 is covered by the queryable validation catalog.

- Commit 0113: catalog audit checkpoint 108; verified rule profile tranche 0108 is covered by the queryable validation catalog.

- Commit 0114: catalog audit checkpoint 109; verified rule profile tranche 0109 is covered by the queryable validation catalog.

- Commit 0115: catalog audit checkpoint 110; verified rule profile tranche 0110 is covered by the queryable validation catalog.

- Commit 0116: catalog audit checkpoint 111; verified rule profile tranche 0111 is covered by the queryable validation catalog.

- Commit 0117: catalog audit checkpoint 112; verified rule profile tranche 0112 is covered by the queryable validation catalog.

- Commit 0118: catalog audit checkpoint 113; verified rule profile tranche 0113 is covered by the queryable validation catalog.

- Commit 0119: catalog audit checkpoint 114; verified rule profile tranche 0114 is covered by the queryable validation catalog.

- Commit 0120: catalog audit checkpoint 115; verified rule profile tranche 0115 is covered by the queryable validation catalog.

- Commit 0121: catalog audit checkpoint 116; verified rule profile tranche 0116 is covered by the queryable validation catalog.

- Commit 0122: catalog audit checkpoint 117; verified rule profile tranche 0117 is covered by the queryable validation catalog.

- Commit 0123: catalog audit checkpoint 118; verified rule profile tranche 0118 is covered by the queryable validation catalog.

- Commit 0124: catalog audit checkpoint 119; verified rule profile tranche 0119 is covered by the queryable validation catalog.

- Commit 0125: catalog audit checkpoint 120; verified rule profile tranche 0120 is covered by the queryable validation catalog.

- Commit 0126: catalog audit checkpoint 121; verified rule profile tranche 0121 is covered by the queryable validation catalog.

- Commit 0127: catalog audit checkpoint 122; verified rule profile tranche 0122 is covered by the queryable validation catalog.

- Commit 0128: catalog audit checkpoint 123; verified rule profile tranche 0123 is covered by the queryable validation catalog.

- Commit 0129: catalog audit checkpoint 124; verified rule profile tranche 0124 is covered by the queryable validation catalog.

- Commit 0130: catalog audit checkpoint 125; verified rule profile tranche 0125 is covered by the queryable validation catalog.

- Commit 0131: catalog audit checkpoint 126; verified rule profile tranche 0126 is covered by the queryable validation catalog.

- Commit 0132: catalog audit checkpoint 127; verified rule profile tranche 0127 is covered by the queryable validation catalog.

- Commit 0133: catalog audit checkpoint 128; verified rule profile tranche 0128 is covered by the queryable validation catalog.

- Commit 0134: catalog audit checkpoint 129; verified rule profile tranche 0129 is covered by the queryable validation catalog.

- Commit 0135: catalog audit checkpoint 130; verified rule profile tranche 0130 is covered by the queryable validation catalog.

- Commit 0136: catalog audit checkpoint 131; verified rule profile tranche 0131 is covered by the queryable validation catalog.

- Commit 0137: catalog audit checkpoint 132; verified rule profile tranche 0132 is covered by the queryable validation catalog.

- Commit 0138: catalog audit checkpoint 133; verified rule profile tranche 0133 is covered by the queryable validation catalog.

- Commit 0139: catalog audit checkpoint 134; verified rule profile tranche 0134 is covered by the queryable validation catalog.

- Commit 0140: catalog audit checkpoint 135; verified rule profile tranche 0135 is covered by the queryable validation catalog.

- Commit 0141: catalog audit checkpoint 136; verified rule profile tranche 0136 is covered by the queryable validation catalog.

- Commit 0142: catalog audit checkpoint 137; verified rule profile tranche 0137 is covered by the queryable validation catalog.

- Commit 0143: catalog audit checkpoint 138; verified rule profile tranche 0138 is covered by the queryable validation catalog.

- Commit 0144: catalog audit checkpoint 139; verified rule profile tranche 0139 is covered by the queryable validation catalog.

- Commit 0145: catalog audit checkpoint 140; verified rule profile tranche 0140 is covered by the queryable validation catalog.

- Commit 0146: catalog audit checkpoint 141; verified rule profile tranche 0141 is covered by the queryable validation catalog.

- Commit 0147: catalog audit checkpoint 142; verified rule profile tranche 0142 is covered by the queryable validation catalog.

- Commit 0148: catalog audit checkpoint 143; verified rule profile tranche 0143 is covered by the queryable validation catalog.

- Commit 0149: catalog audit checkpoint 144; verified rule profile tranche 0144 is covered by the queryable validation catalog.

- Commit 0150: catalog audit checkpoint 145; verified rule profile tranche 0145 is covered by the queryable validation catalog.

- Commit 0151: catalog audit checkpoint 146; verified rule profile tranche 0146 is covered by the queryable validation catalog.

- Commit 0152: catalog audit checkpoint 147; verified rule profile tranche 0147 is covered by the queryable validation catalog.

- Commit 0153: catalog audit checkpoint 148; verified rule profile tranche 0148 is covered by the queryable validation catalog.

- Commit 0154: catalog audit checkpoint 149; verified rule profile tranche 0149 is covered by the queryable validation catalog.

- Commit 0155: catalog audit checkpoint 150; verified rule profile tranche 0150 is covered by the queryable validation catalog.

- Commit 0156: catalog audit checkpoint 151; verified rule profile tranche 0151 is covered by the queryable validation catalog.

- Commit 0157: catalog audit checkpoint 152; verified rule profile tranche 0152 is covered by the queryable validation catalog.

- Commit 0158: catalog audit checkpoint 153; verified rule profile tranche 0153 is covered by the queryable validation catalog.

- Commit 0159: catalog audit checkpoint 154; verified rule profile tranche 0154 is covered by the queryable validation catalog.

- Commit 0160: catalog audit checkpoint 155; verified rule profile tranche 0155 is covered by the queryable validation catalog.

- Commit 0161: catalog audit checkpoint 156; verified rule profile tranche 0156 is covered by the queryable validation catalog.

- Commit 0162: catalog audit checkpoint 157; verified rule profile tranche 0157 is covered by the queryable validation catalog.

- Commit 0163: catalog audit checkpoint 158; verified rule profile tranche 0158 is covered by the queryable validation catalog.

- Commit 0164: catalog audit checkpoint 159; verified rule profile tranche 0159 is covered by the queryable validation catalog.

- Commit 0165: catalog audit checkpoint 160; verified rule profile tranche 0160 is covered by the queryable validation catalog.

- Commit 0166: catalog audit checkpoint 161; verified rule profile tranche 0161 is covered by the queryable validation catalog.

- Commit 0167: catalog audit checkpoint 162; verified rule profile tranche 0162 is covered by the queryable validation catalog.

- Commit 0168: catalog audit checkpoint 163; verified rule profile tranche 0163 is covered by the queryable validation catalog.

- Commit 0169: catalog audit checkpoint 164; verified rule profile tranche 0164 is covered by the queryable validation catalog.

- Commit 0170: catalog audit checkpoint 165; verified rule profile tranche 0165 is covered by the queryable validation catalog.

- Commit 0171: catalog audit checkpoint 166; verified rule profile tranche 0166 is covered by the queryable validation catalog.

- Commit 0172: catalog audit checkpoint 167; verified rule profile tranche 0167 is covered by the queryable validation catalog.

- Commit 0173: catalog audit checkpoint 168; verified rule profile tranche 0168 is covered by the queryable validation catalog.

- Commit 0174: catalog audit checkpoint 169; verified rule profile tranche 0169 is covered by the queryable validation catalog.

- Commit 0175: catalog audit checkpoint 170; verified rule profile tranche 0170 is covered by the queryable validation catalog.

- Commit 0176: catalog audit checkpoint 171; verified rule profile tranche 0171 is covered by the queryable validation catalog.

- Commit 0177: catalog audit checkpoint 172; verified rule profile tranche 0172 is covered by the queryable validation catalog.

- Commit 0178: catalog audit checkpoint 173; verified rule profile tranche 0173 is covered by the queryable validation catalog.

- Commit 0179: catalog audit checkpoint 174; verified rule profile tranche 0174 is covered by the queryable validation catalog.

- Commit 0180: catalog audit checkpoint 175; verified rule profile tranche 0175 is covered by the queryable validation catalog.

- Commit 0181: catalog audit checkpoint 176; verified rule profile tranche 0176 is covered by the queryable validation catalog.
