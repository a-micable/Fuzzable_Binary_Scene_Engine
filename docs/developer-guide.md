# Developer Guide

Development should proceed in small, reviewable commits. Keep parser, serializer, validation,
and tests evolving together so every format change has executable coverage.

## Style

- Use C++17 and avoid compiler-specific extensions in public code.
- Prefer explicit status returns over exceptions at binary parsing boundaries.
- Keep file IO outside core parser and serializer APIs.
- Add regression tests for every parser bug.
- Add fuzz seeds when introducing new binary structures.

## Adding Format Fields

1. Update `docs/binary-format.md`.
2. Extend scene model types.
3. Serialize fields deterministically.
4. Parse fields with bounds checks.
5. Validate cross references and semantic constraints.
6. Add tests and fuzz coverage.
