# Binary Format Specification

All integer fields are little-endian. Strings are encoded as a `u32` byte length followed by raw
UTF-8 bytes without a trailing null byte.

## Header

| Field | Type | Meaning |
| --- | --- | --- |
| magic | 4 bytes | ASCII `BSEN` |
| major | u16 | Format major version |
| minor | u16 | Format minor version |
| flags | u32 | Reserved for future feature negotiation |

## Payload Order

The version 1.0 payload is a deterministic sequence:

1. Scene name and metadata map.
2. Nodes.
3. Textures.
4. Materials.
5. Meshes.
6. Cameras.
7. Lights.
8. Skeletons.
9. Animations.

Each collection starts with a `u32` element count. Object references use `u64` identifiers. The
reserved identifier `0` means "no reference".

## Validation

A valid scene must not use object id `0`, must not duplicate identifiers within an object family,
and must not reference missing nodes, meshes, materials, textures, cameras, or lights. Mesh indices
must refer to existing vertices. Animation channels must target existing nodes, and keyframes must
be ordered within the animation duration.
