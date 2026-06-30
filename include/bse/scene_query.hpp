#pragma once

#include "bse/scene.hpp"

#include <string>
#include <vector>

namespace bse {

enum class QuerySubject {
  kNode,
  kMesh,
  kMaterial,
  kTexture,
  kCamera,
  kLight,
  kAnimation,
  kSkeleton
};

struct QueryResult {
  QuerySubject subject = QuerySubject::kNode;
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::string summary;
};

struct SceneQuery {
  std::string name_contains;
  ObjectId id = kInvalidObjectId;
  QuerySubject subject = QuerySubject::kNode;
  bool include_all_subjects = false;
};

std::vector<QueryResult> QueryScene(const Scene& scene, const SceneQuery& query);
std::vector<QueryResult> QueryByName(const Scene& scene, const std::string& text);
std::vector<QueryResult> QueryById(const Scene& scene, ObjectId id);
std::string FormatQueryResults(const std::vector<QueryResult>& results);
std::string QuerySubjectName(QuerySubject subject);

}  // namespace bse
