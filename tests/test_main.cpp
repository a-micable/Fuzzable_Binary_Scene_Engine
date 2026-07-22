
#include "bse/binary_scene_engine.hpp"
#include "bse/math/aabb.hpp"
#include "bse/mesh/topology.hpp"
#include "bse/material/color_space.hpp"
#include "bse/animation/evaluator.hpp"
#include "bse/scene/hierarchy.hpp"
#include "bse/validation/rules.hpp"
#include "bse/export/write_plan.hpp"
#include "bse/diagnostics/error_reporter.hpp"
#include "bse/catalog/rule_catalog.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

#define REQUIRE(x) do { if(!(x)) throw std::runtime_error(#x); } while(false)
using Test = void(*)();

bse::Scene makeScene() {
    bse::Scene s; s.setName("unit");
    bse::TextureMetadata tex(3,"albedo","albedo.png"); tex.setDimensions(64,64); tex.setFormat(bse::TextureFormat::Rgba8); s.addTexture(tex);
    bse::Material mat(2,"material"); mat.setTextureSlot("baseColor",3); s.addMaterial(mat);
    bse::Mesh mesh(1,"triangle"); mesh.setMaterialId(2);
    mesh.vertices().push_back({{0,0,0},{0,1,0},{0,0}}); mesh.vertices().push_back({{1,0,0},{0,1,0},{1,0}}); mesh.vertices().push_back({{0,1,0},{0,1,0},{0,1}}); mesh.indices()={0,1,2}; s.addMesh(mesh);
    bse::SceneNode node(10,"root"); node.setMeshId(1); s.addNode(node);
    return s;
}
void parser_test(){ auto bytes=bse::serialization::SceneSerializer().serialize(makeScene()); REQUIRE(bytes); auto scene=bse::parser::SceneParser().parse(bytes.value()); REQUIRE(scene); REQUIRE(scene.value().meshes().size()==1); }
void serializer_test(){ auto bytes=bse::serialization::SceneSerializer().serialize(makeScene()); REQUIRE(bytes); REQUIRE(bytes.value().size()>sizeof(bse::format::FileHeader)); }
void animation_test(){ bse::AnimationChannel c(10); c.translations().push_back({0,{1,2,3}}); auto t=c.sample(0.5f); REQUIRE(t.translation.x==1.0f); }
void scene_graph_test(){ bse::Scene s; bse::SceneNode a(1,"a"), b(2,"b"); b.setParentId(1); s.addNode(a); s.addNode(b); s.findNode(1)->children().push_back(2); int n=0; s.traverse([&](const bse::SceneNode&, const bse::Mat4&){++n;}); REQUIRE(n==2); }
void material_test(){ bse::Material m(1,"m"); m.setTextureSlot("normal",7); REQUIRE(m.textureForSlot("normal")==7); }
void compression_test(){ std::vector<std::uint8_t> v(200,42); auto c=bse::compression::compressRle(v); auto d=bse::compression::decompressRle(c,v.size()); REQUIRE(d); REQUIRE(d.value()==v); }
void validation_test(){ auto errors=bse::validation::SceneValidator().validate(makeScene()); REQUIRE(errors.empty()); }

void module_aabb_test(){ auto s=makeScene(); auto box=bse::math::includeMesh(s.meshes()[0]); REQUIRE(!box.empty); REQUIRE(bse::math::contains(box,{0,0,0})); }
void module_topology_test(){ auto stats=bse::mesh::analyzeTopology(makeScene().meshes()[0]); REQUIRE(stats.triangleCount==1); REQUIRE(stats.uniqueEdges==3); }
void module_material_test(){ auto c=bse::material::srgbToLinear(bse::Vec4{1,1,1,1}); REQUIRE(c.x>0.99f); }
void module_animation_eval_test(){ bse::AnimationClip clip(4,"walk"); bse::AnimationChannel ch(10); ch.translations().push_back({0,{5,0,0}}); clip.channels().push_back(ch); auto map=bse::animation::sampleClip(clip,0); REQUIRE(map[10].translation.x==5); }
void module_hierarchy_test(){ bse::Scene s; bse::SceneNode a(1,"a"), b(2,"b"); b.setParentId(1); s.addNode(a); s.addNode(b); s.findNode(1)->children().push_back(2); REQUIRE(bse::scene::hasAncestor(s,2,1)); REQUIRE(bse::scene::depthOf(s,2)==1); }
void module_validation_rules_test(){ auto errors=bse::validation_ext::validateMeshTopology(makeScene()); REQUIRE(errors.empty()); }
void module_export_plan_test(){ auto plan=bse::exporter::estimateWritePlan(makeScene()); REQUIRE(plan.totalBytes>sizeof(bse::format::FileHeader)); }
void module_diagnostics_test(){ bse::ErrorList e; e.add(bse::ErrorCode::ValidationError,"bad thing","test"); auto text=bse::diagnostics::summarizeErrors(e); REQUIRE(text.find("bad thing")!=std::string::npos); }

void catalog_test(){ auto all=bse::catalog::allRuleProfiles(); REQUIRE(all.size()==6250); REQUIRE(all.front().id==1); REQUIRE(all.back().id==6250); auto mesh=bse::catalog::profilesForDomain("mesh"); REQUIRE(!mesh.empty()); auto serious=bse::catalog::profilesAtLeast(bse::catalog::RuleSeverity::Error); REQUIRE(!serious.empty()); auto summary=bse::catalog::summarizeProfiles(); REQUIRE(summary.totalRules==all.size()); REQUIRE(summary.totalWeight>summary.totalRules); REQUIRE(bse::catalog::findRuleProfile(400)!=nullptr); REQUIRE(bse::catalog::findRuleProfile(0)==nullptr); REQUIRE(bse::catalog::findRuleProfile(6251)==nullptr); }

void resource_test(){ bse::resource::ResourceCache c; auto a=c.load<int>("x",[]{return 7;}); auto b=c.load<int>("x",[]{return 9;}); REQUIRE(*a.resource==7); REQUIRE(*b.resource==7); REQUIRE(c.stats().cacheHits==1); }
int main(){ std::vector<std::pair<const char*,Test>> tests={{"parser",parser_test},{"serializer",serializer_test},{"animation",animation_test},{"scene_graph",scene_graph_test},{"material",material_test},{"compression",compression_test},{"validation",validation_test},{"module_aabb",module_aabb_test},{"module_topology",module_topology_test},{"module_material",module_material_test},{"module_animation_eval",module_animation_eval_test},{"module_hierarchy",module_hierarchy_test},{"module_validation_rules",module_validation_rules_test},{"module_export_plan",module_export_plan_test},{"module_diagnostics",module_diagnostics_test},{"catalog",catalog_test},{"resource",resource_test}}; int failed=0; for(auto& t:tests){ try{ t.second(); std::cout<<"[ok] "<<t.first<<"\n"; }catch(const std::exception& e){ ++failed; std::cerr<<"[fail] "<<t.first<<": "<<e.what()<<"\n"; }} return failed?1:0; }
