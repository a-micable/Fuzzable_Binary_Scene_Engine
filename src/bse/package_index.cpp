#include "bse/package_index.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <unordered_set>

namespace bse {

namespace {

std::string Trim(std::string value) {
  auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return {};
  }
  auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1U);
}

std::vector<std::string> Split(const std::string& value, char delimiter) {
  std::vector<std::string> parts;
  std::string current;
  std::istringstream in(value);
  while (std::getline(in, current, delimiter)) {
    parts.push_back(Trim(current));
  }
  return parts;
}

bool EndsWith(const std::string& value, const std::string& suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

Result<std::uint64_t> ParseU64(const std::string& text, const std::string& field) {
  char* end = nullptr;
  const auto value = std::strtoull(text.c_str(), &end, 10);
  if (end == text.c_str() || *end != '\0') {
    return Status::Error(ErrorCode::kMalformedData, "invalid numeric manifest field: " + field);
  }
  return static_cast<std::uint64_t>(value);
}

std::string Escape(const std::string& value) {
  std::string out;
  for (char c : value) {
    if (c == '\\' || c == '|') {
      out.push_back('\\');
    }
    out.push_back(c);
  }
  return out;
}

std::string Unescape(const std::string& value) {
  std::string out;
  bool escaped = false;
  for (char c : value) {
    if (escaped) {
      out.push_back(c);
      escaped = false;
    } else if (c == '\\') {
      escaped = true;
    } else {
      out.push_back(c);
    }
  }
  if (escaped) {
    out.push_back('\\');
  }
  return out;
}

std::vector<std::string> SplitEscapedPipes(const std::string& value) {
  std::vector<std::string> parts;
  std::string current;
  bool escaped = false;
  for (char c : value) {
    if (escaped) {
      current.push_back('\\');
      current.push_back(c);
      escaped = false;
    } else if (c == '\\') {
      escaped = true;
    } else if (c == '|') {
      parts.push_back(Trim(current));
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  if (escaped) {
    current.push_back('\\');
  }
  parts.push_back(Trim(current));
  return parts;
}

}  // namespace

PackageEntryKind GuessPackageEntryKind(const std::string& path) {
  if (EndsWith(path, ".bsen")) {
    return PackageEntryKind::kScene;
  }
  if (EndsWith(path, ".ktx") || EndsWith(path, ".ktx2") || EndsWith(path, ".png") ||
      EndsWith(path, ".jpg") || EndsWith(path, ".jpeg")) {
    return PackageEntryKind::kTexture;
  }
  if (EndsWith(path, ".bmat") || EndsWith(path, ".mat")) {
    return PackageEntryKind::kMaterialLibrary;
  }
  if (EndsWith(path, ".json") || EndsWith(path, ".txt") || EndsWith(path, ".manifest")) {
    return PackageEntryKind::kMetadata;
  }
  return PackageEntryKind::kUnknown;
}

std::string PackageEntryKindName(PackageEntryKind kind) {
  switch (kind) {
    case PackageEntryKind::kScene:
      return "scene";
    case PackageEntryKind::kTexture:
      return "texture";
    case PackageEntryKind::kMaterialLibrary:
      return "material_library";
    case PackageEntryKind::kMetadata:
      return "metadata";
    case PackageEntryKind::kUnknown:
      return "unknown";
  }
  return "unknown";
}

Result<PackageEntryKind> ParsePackageEntryKind(const std::string& name) {
  if (name == "scene") return PackageEntryKind::kScene;
  if (name == "texture") return PackageEntryKind::kTexture;
  if (name == "material_library") return PackageEntryKind::kMaterialLibrary;
  if (name == "metadata") return PackageEntryKind::kMetadata;
  if (name == "unknown") return PackageEntryKind::kUnknown;
  return Status::Error(ErrorCode::kMalformedData, "unknown package entry kind: " + name);
}

std::uint64_t ComputeFnv1a64(const std::vector<std::uint8_t>& bytes) {
  std::uint64_t hash = 1469598103934665603ULL;
  for (std::uint8_t byte : bytes) {
    hash ^= byte;
    hash *= 1099511628211ULL;
  }
  return hash;
}

Result<PackageIndex> ParsePackageManifest(const std::string& text) {
  PackageIndex index;
  std::istringstream in(text);
  std::string line;
  std::size_t line_number = 0;
  while (std::getline(in, line)) {
    ++line_number;
    const auto comment = line.find('#');
    if (comment != std::string::npos) {
      line = line.substr(0, comment);
    }
    line = Trim(line);
    if (line.empty()) {
      continue;
    }
    if (line.rfind("package ", 0) == 0) {
      auto fields = Split(line.substr(8), ' ');
      for (const auto& field : fields) {
        const auto equals = field.find('=');
        if (equals == std::string::npos) {
          continue;
        }
        const auto key = field.substr(0, equals);
        const auto value = field.substr(equals + 1);
        if (key == "name") {
          index.name = Unescape(value);
        } else if (key == "version") {
          auto parsed = ParseU64(value, "version");
          if (!parsed.ok()) return parsed.status();
          index.version = static_cast<std::uint32_t>(parsed.value());
        }
      }
      continue;
    }
    if (line.rfind("entry|", 0) != 0) {
      return Status::Error(ErrorCode::kMalformedData,
                           "manifest line " + std::to_string(line_number) +
                               " must start with package or entry");
    }
    auto parts = SplitEscapedPipes(line);
    if (parts.size() != 6) {
      return Status::Error(ErrorCode::kMalformedData,
                           "manifest line " + std::to_string(line_number) +
                               " must have six pipe-delimited fields");
    }
    PackageEntry entry;
    entry.path = Unescape(parts[1]);
    auto kind = ParsePackageEntryKind(parts[2]);
    auto offset = ParseU64(parts[3], "offset");
    auto size = ParseU64(parts[4], "size");
    auto checksum = ParseU64(parts[5], "checksum");
    if (!kind.ok()) return kind.status();
    if (!offset.ok()) return offset.status();
    if (!size.ok()) return size.status();
    if (!checksum.ok()) return checksum.status();
    entry.kind = kind.value();
    entry.offset = offset.value();
    entry.size = size.value();
    entry.checksum = checksum.value();
    index.entries.push_back(std::move(entry));
  }
  auto valid = ValidatePackageIndex(index);
  if (!valid.ok()) {
    return valid.status();
  }
  return index;
}

std::string WritePackageManifest(const PackageIndex& index) {
  std::ostringstream out;
  out << "package name=" << Escape(index.name) << " version=" << index.version << "\n";
  std::vector<PackageEntry> entries = index.entries;
  std::stable_sort(entries.begin(), entries.end(),
                   [](const PackageEntry& lhs, const PackageEntry& rhs) {
                     if (lhs.offset != rhs.offset) {
                       return lhs.offset < rhs.offset;
                     }
                     return lhs.path < rhs.path;
                   });
  for (const auto& entry : entries) {
    out << "entry|" << Escape(entry.path) << "|" << PackageEntryKindName(entry.kind) << "|"
        << entry.offset << "|" << entry.size << "|" << entry.checksum << "\n";
  }
  return out.str();
}

std::vector<PackageEntry> FindEntriesByKind(const PackageIndex& index, PackageEntryKind kind) {
  std::vector<PackageEntry> out;
  for (const auto& entry : index.entries) {
    if (entry.kind == kind) {
      out.push_back(entry);
    }
  }
  std::stable_sort(out.begin(), out.end(),
                   [](const PackageEntry& lhs, const PackageEntry& rhs) {
                     return lhs.path < rhs.path;
                   });
  return out;
}

Result<PackageEntry> FindEntryByPath(const PackageIndex& index, const std::string& path) {
  for (const auto& entry : index.entries) {
    if (entry.path == path) {
      return entry;
    }
  }
  return Status::Error(ErrorCode::kMalformedData, "package entry not found: " + path);
}

Result<void*> ValidatePackageIndex(const PackageIndex& index) {
  if (index.name.empty()) {
    return Status::Error(ErrorCode::kValidationFailed, "package name must not be empty");
  }
  std::unordered_set<std::string> paths;
  std::vector<PackageEntry> entries = index.entries;
  std::stable_sort(entries.begin(), entries.end(),
                   [](const PackageEntry& lhs, const PackageEntry& rhs) {
                     return lhs.offset < rhs.offset;
                   });
  std::uint64_t end = 0;
  for (const auto& entry : entries) {
    if (entry.path.empty()) {
      return Status::Error(ErrorCode::kValidationFailed, "package entry path must not be empty");
    }
    if (!paths.insert(entry.path).second) {
      return Status::Error(ErrorCode::kValidationFailed,
                           "duplicate package entry path: " + entry.path);
    }
    if (entry.offset < end) {
      return Status::Error(ErrorCode::kValidationFailed,
                           "package entries overlap around: " + entry.path);
    }
    end = entry.offset + entry.size;
    if (end < entry.offset) {
      return Status::Error(ErrorCode::kValidationFailed,
                           "package entry range overflows: " + entry.path);
    }
  }
  return static_cast<void*>(nullptr);
}

}  // namespace bse
