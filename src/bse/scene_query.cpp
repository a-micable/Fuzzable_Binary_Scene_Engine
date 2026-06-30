#include "bse/scene_query.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace bse {

namespace {

std::string Lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return value;
}

bool MatchesName(const std::string& name, const std::string& needle) {
  if (needle.empty()) {
    return true;
  }
  return Lower(name).find(Lower(needle)) != std::string::npos;
}

bool MatchesId(ObjectId candidate, ObjectId requested) {
  return requested == kInvalidObjectId || candidate == requested;
}

void Add(std::vector<QueryResult>* results, QuerySubject subject, ObjectId id,
         const std::string& name, std::string summary) {
  results->push_back({subject, id, name, std::move(summary)});
}

void QueryNodes(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& node : scene.nodes) {
    if (!MatchesId(node.id, query.id) || !MatchesName(node.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "parent=" << node.parent << " mesh=" << node.mesh << " camera=" << node.camera
            << " light=" << node.light << " children=" << node.children.size();
    Add(results, QuerySubject::kNode, node.id, node.name, summary.str());
  }
}

void QueryMeshes(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& mesh : scene.meshes) {
    if (!MatchesId(mesh.id, query.id) || !MatchesName(mesh.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "material=" << mesh.material << " vertices=" << mesh.vertices.size()
            << " indices=" << mesh.indices.size();
    Add(results, QuerySubject::kMesh, mesh.id, mesh.name, summary.str());
  }
}

void QueryMaterials(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& material : scene.materials) {
    if (!MatchesId(material.id, query.id) || !MatchesName(material.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "texture=" << material.base_color_texture << " roughness=" << material.roughness
            << " metallic=" << material.metallic;
    Add(results, QuerySubject::kMaterial, material.id, material.name, summary.str());
  }
}

void QueryTextures(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& texture : scene.textures) {
    if (!MatchesId(texture.id, query.id) || !MatchesName(texture.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << texture.width << "x" << texture.height << " " << texture.color_space << " "
            << texture.uri;
    Add(results, QuerySubject::kTexture, texture.id, texture.name, summary.str());
  }
}

void QueryCameras(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& camera : scene.cameras) {
    if (!MatchesId(camera.id, query.id) || !MatchesName(camera.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "fov=" << camera.vertical_fov_degrees << " near=" << camera.near_plane
            << " far=" << camera.far_plane;
    Add(results, QuerySubject::kCamera, camera.id, camera.name, summary.str());
  }
}

void QueryLights(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& light : scene.lights) {
    if (!MatchesId(light.id, query.id) || !MatchesName(light.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "type=" << static_cast<int>(light.type) << " intensity=" << light.intensity;
    Add(results, QuerySubject::kLight, light.id, light.name, summary.str());
  }
}

void QueryAnimations(const Scene& scene, const SceneQuery& query,
                     std::vector<QueryResult>* results) {
  for (const auto& animation : scene.animations) {
    if (!MatchesId(animation.id, query.id) || !MatchesName(animation.name, query.name_contains)) {
      continue;
    }
    std::size_t keys = 0;
    for (const auto& channel : animation.channels) {
      keys += channel.keys.size();
    }
    std::ostringstream summary;
    summary << "duration=" << animation.duration_seconds << " channels="
            << animation.channels.size() << " keys=" << keys;
    Add(results, QuerySubject::kAnimation, animation.id, animation.name, summary.str());
  }
}

void QuerySkeletons(const Scene& scene, const SceneQuery& query, std::vector<QueryResult>* results) {
  for (const auto& skeleton : scene.skeletons) {
    if (!MatchesId(skeleton.id, query.id) || !MatchesName(skeleton.name, query.name_contains)) {
      continue;
    }
    std::ostringstream summary;
    summary << "joints=" << skeleton.joints.size();
    Add(results, QuerySubject::kSkeleton, skeleton.id, skeleton.name, summary.str());
  }
}

}  // namespace

std::string QuerySubjectName(QuerySubject subject) {
  switch (subject) {
    case QuerySubject::kNode:
      return "node";
    case QuerySubject::kMesh:
      return "mesh";
    case QuerySubject::kMaterial:
      return "material";
    case QuerySubject::kTexture:
      return "texture";
    case QuerySubject::kCamera:
      return "camera";
    case QuerySubject::kLight:
      return "light";
    case QuerySubject::kAnimation:
      return "animation";
    case QuerySubject::kSkeleton:
      return "skeleton";
  }
  return "unknown";
}

std::vector<QueryResult> QueryScene(const Scene& scene, const SceneQuery& query) {
  std::vector<QueryResult> results;
  auto run = [&](QuerySubject subject) {
    SceneQuery scoped = query;
    scoped.subject = subject;
    switch (subject) {
      case QuerySubject::kNode:
        QueryNodes(scene, scoped, &results);
        break;
      case QuerySubject::kMesh:
        QueryMeshes(scene, scoped, &results);
        break;
      case QuerySubject::kMaterial:
        QueryMaterials(scene, scoped, &results);
        break;
      case QuerySubject::kTexture:
        QueryTextures(scene, scoped, &results);
        break;
      case QuerySubject::kCamera:
        QueryCameras(scene, scoped, &results);
        break;
      case QuerySubject::kLight:
        QueryLights(scene, scoped, &results);
        break;
      case QuerySubject::kAnimation:
        QueryAnimations(scene, scoped, &results);
        break;
      case QuerySubject::kSkeleton:
        QuerySkeletons(scene, scoped, &results);
        break;
    }
  };
  if (query.include_all_subjects) {
    run(QuerySubject::kNode);
    run(QuerySubject::kMesh);
    run(QuerySubject::kMaterial);
    run(QuerySubject::kTexture);
    run(QuerySubject::kCamera);
    run(QuerySubject::kLight);
    run(QuerySubject::kAnimation);
    run(QuerySubject::kSkeleton);
  } else {
    run(query.subject);
  }
  std::stable_sort(results.begin(), results.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.subject != rhs.subject) {
      return static_cast<int>(lhs.subject) < static_cast<int>(rhs.subject);
    }
    return lhs.id < rhs.id;
  });
  return results;
}

std::vector<QueryResult> QueryByName(const Scene& scene, const std::string& text) {
  SceneQuery query;
  query.include_all_subjects = true;
  query.name_contains = text;
  return QueryScene(scene, query);
}

std::vector<QueryResult> QueryById(const Scene& scene, ObjectId id) {
  SceneQuery query;
  query.include_all_subjects = true;
  query.id = id;
  return QueryScene(scene, query);
}

std::string FormatQueryResults(const std::vector<QueryResult>& results) {
  std::ostringstream out;
  for (const auto& result : results) {
    out << QuerySubjectName(result.subject) << " " << result.id << " " << result.name << " "
        << result.summary << "\n";
  }
  return out.str();
}

}  // namespace bse
