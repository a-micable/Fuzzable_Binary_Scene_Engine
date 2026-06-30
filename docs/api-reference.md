# API Reference

The public API is intentionally buffer-oriented.

## Scene Model

`bse::Scene` owns metadata, nodes, meshes, materials, textures, animations, skeletons, cameras,
and lights. References between objects use `bse::ObjectId`; `bse::kInvalidObjectId` represents
an absent reference.

## Parsing

`bse::ParseScene(const std::uint8_t*, std::size_t)` and `bse::ParseScene(const std::vector<std::uint8_t>&)`
return `bse::Result<bse::Scene>`.

## Serialization

`bse::SerializeScene(const bse::Scene&)` validates a scene and returns serialized bytes.

## Validation

`bse::ValidateScene` accepts optional `bse::DiagnosticSink` and `bse::ValidationLimits` arguments.

## Compression

`bse::CompressRle` and `bse::DecompressRle` provide a simple byte-run codec used by fuzz targets
and future compressed sections.
