#include "bse/parser.hpp"
#include "bse/package_index.hpp"
#include "bse/scene_diff.hpp"
#include "bse/scene_lint.hpp"
#include "bse/scene_ops.hpp"
#include "bse/scene_query.hpp"
#include "bse/scene_stats.hpp"
#include "bse/serializer.hpp"
#include "bse/scene.hpp"
#include "bse/text_scene.hpp"
#include "bse/validator.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

bse::Result<std::vector<std::uint8_t>> ReadFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return bse::Status::Error(bse::ErrorCode::kIoError, "failed to open " + path);
  }
  std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(in),
                                  std::istreambuf_iterator<char>()};
  if (!in.eof() && in.fail()) {
    return bse::Status::Error(bse::ErrorCode::kIoError, "failed to read " + path);
  }
  return bytes;
}

bool WriteFile(const std::string& path, const std::vector<std::uint8_t>& bytes) {
  std::ofstream out(path, std::ios::binary);
  out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return out.good();
}

bool WriteTextFile(const std::string& path, const std::string& text) {
  std::ofstream out(path, std::ios::binary);
  out << text;
  return out.good();
}

std::string BytesToText(const std::vector<std::uint8_t>& bytes) {
  return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void PrintUsage() {
  std::cerr
      << "usage:\n"
      << "  bsectl create-minimal <out.bsen>\n"
      << "  bsectl create-sample <out.bsen>\n"
      << "  bsectl inspect <scene.bsen>\n"
      << "  bsectl validate <scene.bsen>\n"
      << "  bsectl stats <scene.bsen>\n"
      << "  bsectl audit <scene.bsen>\n"
      << "  bsectl lint <scene.bsen>\n"
      << "  bsectl find <scene.bsen> <name-fragment>\n"
      << "  bsectl normalize <in.bsen> <out.bsen>\n"
      << "  bsectl export-text <in.bsen> <out.bsetxt>\n"
      << "  bsectl import-text <in.bsetxt> <out.bsen>\n"
      << "  bsectl diff <left.bsen> <right.bsen>\n"
      << "  bsectl validate-manifest <manifest.txt>\n";
}

bse::Scene MakeSampleScene() {
  bse::Scene scene = bse::MakeMinimalScene("sample");
  scene.metadata.emplace("authoring_tool", "bsectl");
  scene.metadata.emplace("purpose", "sample scene");

  scene.textures.push_back({10, "checker_albedo", "textures/checker.ktx2", 256, 256, "srgb"});
  scene.materials.push_back({20, "painted_matte", {0.8F, 0.25F, 0.15F}, 0.55F, 0.0F, 10});

  bse::Mesh mesh;
  mesh.id = 30;
  mesh.name = "triangle";
  mesh.material = 20;
  mesh.vertices.push_back({{-0.5F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}});
  mesh.vertices.push_back({{0.5F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 0.0F}});
  mesh.vertices.push_back({{0.0F, 0.75F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.5F, 1.0F}});
  mesh.indices = {0, 1, 2};
  scene.meshes.push_back(mesh);
  scene.nodes.front().mesh = 30;

  scene.cameras.push_back({40, "main_camera", 60.0F, 0.05F, 250.0F});
  bse::Node camera_node;
  camera_node.id = 2;
  camera_node.name = "camera";
  camera_node.camera = 40;
  camera_node.parent = 1;
  camera_node.local.translation = {0.0F, -3.0F, 2.0F};
  scene.nodes.push_back(camera_node);
  scene.nodes.front().children.push_back(2);

  scene.lights.push_back(
      {50, "key_light", bse::LightType::kDirectional, {1.0F, 0.95F, 0.9F}, 2.0F});
  bse::Node light_node;
  light_node.id = 3;
  light_node.name = "key_light";
  light_node.light = 50;
  light_node.parent = 1;
  scene.nodes.push_back(light_node);
  scene.nodes.front().children.push_back(3);

  bse::Animation animation;
  animation.id = 60;
  animation.name = "root_lift";
  animation.duration_seconds = 1.0F;
  bse::AnimationChannel channel;
  channel.target_node = 1;
  channel.keys.push_back({0.0F, {}});
  bse::Transform lifted;
  lifted.translation = {0.0F, 0.25F, 0.0F};
  channel.keys.push_back({1.0F, lifted});
  animation.channels.push_back(channel);
  scene.animations.push_back(animation);

  return scene;
}

bse::Result<bse::Scene> ReadScene(const std::string& path) {
  auto bytes = ReadFile(path);
  if (!bytes.ok()) {
    return bytes.status();
  }
  return bse::ParseScene(bytes.value());
}

int WriteScene(const std::string& path, const bse::Scene& scene) {
  auto bytes = bse::SerializeScene(scene);
  if (!bytes.ok()) {
    std::cerr << bytes.status().message() << "\n";
    return 1;
  }
  if (!WriteFile(path, bytes.value())) {
    std::cerr << "failed to write " << path << "\n";
    return 1;
  }
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    PrintUsage();
    return 2;
  }

  const std::string command = argv[1];
  const std::string path = argv[2];

  if (command == "create-minimal" || command == "create-sample") {
    const auto scene =
        command == "create-minimal" ? bse::MakeMinimalScene("minimal") : MakeSampleScene();
    auto bytes = bse::SerializeScene(scene);
    if (!bytes.ok()) {
      std::cerr << bytes.status().message() << "\n";
      return 1;
    }
    if (!WriteFile(path, bytes.value())) {
      std::cerr << "failed to write " << path << "\n";
      return 1;
    }
    return 0;
  }

  if (command == "import-text") {
    if (argc != 4) {
      PrintUsage();
      return 2;
    }
    auto bytes = ReadFile(path);
    if (!bytes.ok()) {
      std::cerr << bytes.status().message() << "\n";
      return 1;
    }
    auto scene = bse::ParseTextScene(BytesToText(bytes.value()));
    if (!scene.ok()) {
      std::cerr << scene.status().message() << "\n";
      return 1;
    }
    return WriteScene(argv[3], scene.value());
  }

  if (command == "validate-manifest") {
    auto bytes = ReadFile(path);
    if (!bytes.ok()) {
      std::cerr << bytes.status().message() << "\n";
      return 1;
    }
    auto index = bse::ParsePackageManifest(BytesToText(bytes.value()));
    if (!index.ok()) {
      std::cerr << index.status().message() << "\n";
      return 1;
    }
    std::cout << "package: " << index.value().name << "\n";
    std::cout << "entries: " << index.value().entries.size() << "\n";
    return 0;
  }

  if (command == "diff") {
    if (argc != 4) {
      PrintUsage();
      return 2;
    }
    auto left = ReadScene(path);
    auto right = ReadScene(argv[3]);
    if (!left.ok()) {
      std::cerr << left.status().message() << "\n";
      return 1;
    }
    if (!right.ok()) {
      std::cerr << right.status().message() << "\n";
      return 1;
    }
    auto diff = bse::DiffScenes(left.value(), right.value());
    std::cout << bse::FormatDiff(diff);
    return diff.empty() ? 0 : 1;
  }

  auto scene = ReadScene(path);
  if (!scene.ok()) {
    std::cerr << scene.status().message() << "\n";
    return 1;
  }

  if (command == "validate") {
    bse::DiagnosticSink diagnostics;
    auto status = bse::ValidateScene(scene.value(), &diagnostics);
    for (const auto& diagnostic : diagnostics.diagnostics()) {
      std::cerr << diagnostic.code << ": " << diagnostic.message << "\n";
    }
    return status.ok() ? 0 : 1;
  }

  if (command == "stats") {
    std::cout << bse::FormatSceneStats(bse::ComputeSceneStats(scene.value()));
    auto bounds = bse::ComputeSceneBounds(scene.value());
    if (bounds.valid) {
      std::cout << "bounds_min=" << bounds.min.x << "," << bounds.min.y << "," << bounds.min.z
                << "\n";
      std::cout << "bounds_max=" << bounds.max.x << "," << bounds.max.y << "," << bounds.max.z
                << "\n";
    }
    return 0;
  }

  if (command == "audit") {
    auto issues = bse::AuditSceneStructure(scene.value());
    for (const auto& issue : issues) {
      std::cout << issue.code << " " << issue.subject << ": " << issue.message << "\n";
    }
    return issues.empty() ? 0 : 1;
  }

  if (command == "lint") {
    auto findings = bse::LintScene(scene.value());
    std::cout << bse::FormatLintFindings(findings);
    return bse::HasLintErrors(findings) ? 1 : 0;
  }

  if (command == "find") {
    if (argc != 4) {
      PrintUsage();
      return 2;
    }
    auto results = bse::QueryByName(scene.value(), argv[3]);
    std::cout << bse::FormatQueryResults(results);
    return results.empty() ? 1 : 0;
  }

  if (command == "normalize") {
    if (argc != 4) {
      PrintUsage();
      return 2;
    }
    auto normalized = scene.value();
    bse::NormalizeOptions options;
    options.drop_unreferenced_resources = true;
    bse::NormalizeScene(&normalized, options);
    return WriteScene(argv[3], normalized);
  }

  if (command == "export-text") {
    if (argc != 4) {
      PrintUsage();
      return 2;
    }
    auto text = bse::WriteTextScene(scene.value());
    if (!text.ok()) {
      std::cerr << text.status().message() << "\n";
      return 1;
    }
    if (!WriteTextFile(argv[3], text.value())) {
      std::cerr << "failed to write " << argv[3] << "\n";
      return 1;
    }
    return 0;
  }

  if (command == "inspect") {
    std::cout << "scene: " << scene.value().name << "\n";
    std::cout << "nodes: " << scene.value().nodes.size() << "\n";
    std::cout << "meshes: " << scene.value().meshes.size() << "\n";
    std::cout << "materials: " << scene.value().materials.size() << "\n";
    std::cout << "textures: " << scene.value().textures.size() << "\n";
    std::cout << "animations: " << scene.value().animations.size() << "\n";
    return 0;
  }

  PrintUsage();
  return 2;
}
