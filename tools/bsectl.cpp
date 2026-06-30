#include "bse/parser.hpp"
#include "bse/serializer.hpp"
#include "bse/scene.hpp"
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

void PrintUsage() {
  std::cerr << "usage: bsectl <inspect|validate|create-minimal|create-sample> <path>\n";
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

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
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

  auto bytes = ReadFile(path);
  if (!bytes.ok()) {
    std::cerr << bytes.status().message() << "\n";
    return 1;
  }
  auto scene = bse::ParseScene(bytes.value());
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
