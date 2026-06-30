#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace bse {

struct GraphNodeInfo {
  ObjectId id = kInvalidObjectId;
  ObjectId parent = kInvalidObjectId;
  std::vector<ObjectId> children;
  std::size_t depth = 0;
  bool reachable = false;
};

struct SceneGraph {
  std::unordered_map<ObjectId, GraphNodeInfo> nodes;
  std::vector<ObjectId> roots;
  std::vector<ObjectId> unreachable;
  std::vector<std::vector<ObjectId>> cycles;
};

struct GraphTraversalStep {
  ObjectId id = kInvalidObjectId;
  std::size_t depth = 0;
  std::string path;
};

struct SceneGraphSummary {
  std::size_t node_count = 0;
  std::size_t root_count = 0;
  std::size_t edge_count = 0;
  std::size_t leaf_count = 0;
  std::size_t unreachable_count = 0;
  std::size_t cycle_count = 0;
  std::size_t max_depth = 0;
};

struct GraphDepthBucket {
  std::size_t depth = 0;
  std::size_t count = 0;
};

SceneGraph BuildSceneGraph(const Scene& scene);
std::vector<GraphTraversalStep> TraverseDepthFirst(const SceneGraph& graph);
std::vector<GraphTraversalStep> TraverseBreadthFirst(const SceneGraph& graph);
std::vector<ObjectId> PathToRoot(const SceneGraph& graph, ObjectId id);
std::vector<ObjectId> DescendantsOf(const SceneGraph& graph, ObjectId id);
std::vector<ObjectId> AncestorsOf(const SceneGraph& graph, ObjectId id);
std::vector<ObjectId> LeafNodes(const SceneGraph& graph);
std::vector<ObjectId> NodesAtDepth(const SceneGraph& graph, std::size_t depth);
std::vector<ObjectId> SiblingNodes(const SceneGraph& graph, ObjectId id);
std::vector<ObjectId> ReachableFrom(const SceneGraph& graph, ObjectId id);
std::vector<std::vector<ObjectId>> ConnectedComponents(const SceneGraph& graph);
std::vector<GraphDepthBucket> GraphDepthHistogram(const SceneGraph& graph);
std::size_t SubtreeSize(const SceneGraph& graph, ObjectId id);
ObjectId LowestCommonAncestor(const SceneGraph& graph, ObjectId left, ObjectId right);
bool GraphContainsNode(const SceneGraph& graph, ObjectId id);
bool GraphHasRoots(const SceneGraph& graph);
bool GraphHasCycles(const SceneGraph& graph);
bool GraphHasUnreachableNodes(const SceneGraph& graph);
bool GraphIsForest(const SceneGraph& graph);
std::size_t GraphEdgeCount(const SceneGraph& graph);
std::size_t GraphRootCount(const SceneGraph& graph);
Result<std::vector<ObjectId>> TopologicalNodeOrder(const SceneGraph& graph);
SceneGraphSummary SummarizeSceneGraph(const SceneGraph& graph);
std::string FormatSceneGraphSummary(const SceneGraphSummary& summary);
std::string FormatSceneGraph(const SceneGraph& graph);
std::string WriteGraphDot(const SceneGraph& graph);
std::string WriteGraphAdjacencyList(const SceneGraph& graph);
std::string WriteGraphCsv(const SceneGraph& graph);
std::string WriteGraphJson(const SceneGraph& graph);
std::string FormatGraphTraversal(const std::vector<GraphTraversalStep>& steps);
std::vector<std::string> ValidateGraphConsistency(const SceneGraph& graph);

}  // namespace bse
