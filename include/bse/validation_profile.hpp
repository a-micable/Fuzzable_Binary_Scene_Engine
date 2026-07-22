#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"
#include "bse/validator.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bse {

enum class ProfileSeverity {
  kInfo,
  kWarning,
  kError
};

struct ValidationProfile {
  std::string name;
  ValidationLimits limits;
  std::size_t max_depth = 128;
  std::size_t max_children_per_node = 4096;
  std::size_t max_materials = 1'000'000;
  std::size_t max_textures = 1'000'000;
  std::size_t max_animation_channels = 1'000'000;
  std::size_t max_animation_keys = 8'000'000;
  std::size_t max_metadata_entries = 4096;
  std::size_t max_name_bytes = 1024;
  bool require_root_node = true;
  bool require_reachable_nodes = true;
  bool require_indexed_triangles = false;
  bool allow_empty_meshes = true;
  bool allow_missing_materials = false;
};

struct ProfileFinding {
  ProfileSeverity severity = ProfileSeverity::kInfo;
  std::string code;
  std::string subject;
  std::string message;
};

struct ProfileReport {
  std::string profile_name;
  std::vector<ProfileFinding> findings;
};

ValidationProfile DesktopRuntimeProfile();
ValidationProfile MobileRuntimeProfile();
ValidationProfile StrictAssetProfile();
ValidationProfile FuzzingProfile();
std::vector<ValidationProfile> BuiltInValidationProfiles();
ProfileReport CheckValidationProfile(const Scene& scene, const ValidationProfile& profile);
bool ProfileReportHasErrors(const ProfileReport& report);
std::string ProfileSeverityName(ProfileSeverity severity);
std::string FormatProfileReport(const ProfileReport& report);

}  // namespace bse
