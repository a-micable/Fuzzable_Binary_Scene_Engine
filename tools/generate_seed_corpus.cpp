
#include "bse/binary_scene_engine.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

bse::Mesh makeTriangle(bse::ObjectId id, const std::string& name, bse::ObjectId materialId, float offset) {
    bse::Mesh mesh(id, name);
    mesh.setMaterialId(materialId);
    mesh.vertices().push_back({{offset, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});
    mesh.vertices().push_back({{offset + 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}});
    mesh.vertices().push_back({{offset, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});
    mesh.indices() = {0, 1, 2};
    return mesh;
}

bse::Scene makeBaseScene(const std::string& name) {
    bse::Scene scene;
    scene.setName(name);

    bse::TextureMetadata texture(100, "albedo", "textures/albedo.png");
    texture.setDimensions(128, 128);
    texture.setFormat(bse::TextureFormat::Rgba8);
    scene.addTexture(texture);

    bse::Material material(200, "painted_plastic");
    material.setBaseColor({0.8f, 0.7f, 0.6f, 1.0f});
    material.setRoughness(0.45f);
    material.setTextureSlot("baseColor", texture.id());
    scene.addMaterial(material);
    return scene;
}

void writeScene(const std::filesystem::path& path, const bse::Scene& scene) {
    auto bytes = bse::serialization::SceneSerializer().serialize(scene);
    if (!bytes) {
        throw std::runtime_error(bytes.error().message);
    }
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.value().data()), static_cast<std::streamsize>(bytes.value().size()));
}

bse::Scene makeNestedScene() {
    auto scene = makeBaseScene("nested_scene_graph");
    scene.addMesh(makeTriangle(1, "root_mesh", 200, 0.0f));
    bse::SceneNode root(10, "root");
    root.setMeshId(1);
    scene.addNode(root);
    for (int i = 0; i < 8; ++i) {
        bse::SceneNode child(11 + static_cast<bse::ObjectId>(i), "child_" + std::to_string(i));
        child.setParentId(i == 0 ? 10 : 10 + static_cast<bse::ObjectId>(i));
        child.localTransform().translation = {static_cast<float>(i), 0.0f, 0.0f};
        scene.addNode(child);
    }
    for (auto& node : scene.nodes()) {
        if (node.id() > 10) {
            if (auto* parent = scene.findNode(node.parentId())) {
                parent->children().push_back(node.id());
            }
        }
    }
    return scene;
}

bse::Scene makeAnimatedScene() {
    auto scene = makeBaseScene("animated_model");
    scene.addMesh(makeTriangle(1, "animated_triangle", 200, 0.0f));
    bse::SceneNode node(10, "animated_root");
    node.setMeshId(1);
    scene.addNode(node);

    bse::AnimationClip clip(300, "bounce");
    clip.setDuration(1.0f);
    bse::AnimationChannel channel(10);
    channel.translations().push_back({0.0f, {0.0f, 0.0f, 0.0f}});
    channel.translations().push_back({0.5f, {0.0f, 2.0f, 0.0f}});
    channel.translations().push_back({1.0f, {0.0f, 0.0f, 0.0f}});
    clip.channels().push_back(channel);
    scene.addAnimation(clip);
    return scene;
}

bse::Scene makeLargeScene() {
    auto scene = makeBaseScene("large_scene");
    for (int i = 0; i < 64; ++i) {
        const auto meshId = static_cast<bse::ObjectId>(1000 + i);
        scene.addMesh(makeTriangle(meshId, "mesh_" + std::to_string(i), 200, static_cast<float>(i)));
        bse::SceneNode node(static_cast<bse::ObjectId>(2000 + i), "node_" + std::to_string(i));
        node.setMeshId(meshId);
        node.localTransform().translation = {static_cast<float>(i % 8), 0.0f, static_cast<float>(i / 8)};
        scene.addNode(node);
    }
    return scene;
}

} // namespace

int main(int argc, char** argv) {
    const std::filesystem::path output = argc > 1 ? argv[1] : "fuzz/seed_corpus";
    std::filesystem::create_directories(output);

    writeScene(output / "generated_nested_scene.bsen", makeNestedScene());
    writeScene(output / "generated_animated_model.bsen", makeAnimatedScene());
    writeScene(output / "generated_large_scene.bsen", makeLargeScene());

    std::cout << "wrote generated seed corpus to " << output << '\n';
    return 0;
}
