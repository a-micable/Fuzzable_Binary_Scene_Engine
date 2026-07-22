#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace bse {

constexpr std::array<std::uint8_t, 4> kSceneMagic{{'B', 'S', 'E', 'N'}};
constexpr std::uint16_t kFormatMajor = 1;
constexpr std::uint16_t kFormatMinor = 0;
constexpr std::size_t kMaxStringBytes = 64 * 1024;
constexpr std::size_t kMaxSectionBytes = 128 * 1024 * 1024;

enum class SectionType : std::uint16_t {
  kMetadata = 1,
  kNodes = 2,
  kMeshes = 3,
  kMaterials = 4,
  kTextures = 5,
  kAnimations = 6,
  kSkeletons = 7,
  kCameras = 8,
  kLights = 9
};

struct FileHeader {
  std::uint16_t major = kFormatMajor;
  std::uint16_t minor = kFormatMinor;
  std::uint32_t flags = 0;
  std::uint32_t section_count = 0;
};

struct SectionHeader {
  SectionType type = SectionType::kMetadata;
  std::uint32_t version = 1;
  std::uint64_t size = 0;
};

const char* SectionName(SectionType type);

}  // namespace bse
