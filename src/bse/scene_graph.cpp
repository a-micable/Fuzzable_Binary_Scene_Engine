#include "bse/scene_graph.hpp"

#include <algorithm>
#include <deque>
#include <sstream>
#include <unordered_set>

namespace bse {

namespace {

void SortIds(std::vector<ObjectId>* ids) {
  std::sort(ids->begin(), ids->end());
  ids->erase(std::unique(ids->begin(), ids->end()), ids->end());
}

void MarkReachable(SceneGraph* graph, ObjectId id, std::size_t depth) {
  auto found = graph->nodes.find(id);
  if (found == graph->nodes.end()) return;
  if (found->second.reachable && found->second.depth <= depth) return;
  found->second.reachable = true;
  found->second.depth = depth;
  for (ObjectId child : found->second.children) {
    MarkReachable(graph, child, depth + 1U);
  }
}

void FindCyclesFrom(const SceneGraph& graph, ObjectId id, std::vector<ObjectId>* stack,
                    std::unordered_set<ObjectId>* visiting, std::unordered_set<ObjectId>* visited,
                    std::vector<std::vector<ObjectId>>* cycles) {
  if (visited->find(id) != visited->end()) return;
  if (visiting->find(id) != visiting->end()) {
    auto iter = std::find(stack->begin(), stack->end(), id);
    if (iter != stack->end()) cycles->push_back(std::vector<ObjectId>(iter, stack->end()));
    return;
  }
  visiting->insert(id);
  stack->push_back(id);
  auto found = graph.nodes.find(id);
  if (found != graph.nodes.end()) {
    for (ObjectId child : found->second.children) {
      FindCyclesFrom(graph, child, stack, visiting, visited, cycles);
    }
  }
  stack->pop_back();
  visiting->erase(id);
  visited->insert(id);
}

std::vector<std::vector<ObjectId>> FindCycles(const SceneGraph& graph) {
  std::vector<std::vector<ObjectId>> cycles;
  std::vector<ObjectId> stack;
  std::unordered_set<ObjectId> visiting;
  std::unordered_set<ObjectId> visited;
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (ObjectId id : ids) {
    FindCyclesFrom(graph, id, &stack, &visiting, &visited, &cycles);
  }
  return cycles;
}

std::string JoinPath(const std::vector<ObjectId>& ids) {
  std::ostringstream out;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    if (i != 0) out << "/";
    out << ids[i];
  }
  return out.str();
}

void TraverseDepthFirstFrom(const SceneGraph& graph, ObjectId id, std::vector<ObjectId>* path,
                            std::vector<GraphTraversalStep>* steps) {
  auto found = graph.nodes.find(id);
  if (found == graph.nodes.end()) return;
  path->push_back(id);
  steps->push_back({id, path->size() - 1U, JoinPath(*path)});
  for (ObjectId child : found->second.children) {
    TraverseDepthFirstFrom(graph, child, path, steps);
  }
  path->pop_back();
}

}  // namespace

SceneGraph BuildSceneGraph(const Scene& scene) {
  SceneGraph graph;
  for (const auto& node : scene.nodes) {
    GraphNodeInfo info;
    info.id = node.id;
    info.parent = node.parent;
    info.children = node.children;
    SortIds(&info.children);
    graph.nodes.emplace(node.id, info);
  }
  for (const auto& node : scene.nodes) {
    if (node.parent == kInvalidObjectId || graph.nodes.find(node.parent) == graph.nodes.end()) {
      graph.roots.push_back(node.id);
      continue;
    }
    auto parent = graph.nodes.find(node.parent);
    if (parent != graph.nodes.end()) parent->second.children.push_back(node.id);
  }
  SortIds(&graph.roots);
  for (auto& entry : graph.nodes) SortIds(&entry.second.children);
  for (ObjectId root : graph.roots) MarkReachable(&graph, root, 0);
  for (const auto& entry : graph.nodes) {
    if (!entry.second.reachable) graph.unreachable.push_back(entry.first);
  }
  SortIds(&graph.unreachable);
  graph.cycles = FindCycles(graph);
  return graph;
}

std::vector<GraphTraversalStep> TraverseDepthFirst(const SceneGraph& graph) {
  std::vector<GraphTraversalStep> steps;
  std::vector<ObjectId> path;
  for (ObjectId root : graph.roots) TraverseDepthFirstFrom(graph, root, &path, &steps);
  return steps;
}

std::vector<GraphTraversalStep> TraverseBreadthFirst(const SceneGraph& graph) {
  std::vector<GraphTraversalStep> steps;
  std::deque<std::vector<ObjectId>> queue;
  for (ObjectId root : graph.roots) queue.push_back({root});
  while (!queue.empty()) {
    auto path = queue.front();
    queue.pop_front();
    const ObjectId id = path.back();
    steps.push_back({id, path.size() - 1U, JoinPath(path)});
    auto found = graph.nodes.find(id);
    if (found == graph.nodes.end()) continue;
    for (ObjectId child : found->second.children) {
      auto child_path = path;
      child_path.push_back(child);
      queue.push_back(std::move(child_path));
    }
  }
  return steps;
}

std::vector<ObjectId> PathToRoot(const SceneGraph& graph, ObjectId id) {
  std::vector<ObjectId> path;
  std::unordered_set<ObjectId> seen;
  ObjectId cursor = id;
  while (cursor != kInvalidObjectId && seen.insert(cursor).second) {
    auto found = graph.nodes.find(cursor);
    if (found == graph.nodes.end()) break;
    path.push_back(cursor);
    cursor = found->second.parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<ObjectId> DescendantsOf(const SceneGraph& graph, ObjectId id) {
  std::vector<ObjectId> descendants;
  auto found = graph.nodes.find(id);
  if (found == graph.nodes.end()) return descendants;
  std::deque<ObjectId> queue(found->second.children.begin(), found->second.children.end());
  while (!queue.empty()) {
    ObjectId current = queue.front();
    queue.pop_front();
    descendants.push_back(current);
    auto child = graph.nodes.find(current);
    if (child == graph.nodes.end()) continue;
    for (ObjectId grandchild : child->second.children) queue.push_back(grandchild);
  }
  SortIds(&descendants);
  return descendants;
}

std::vector<ObjectId> AncestorsOf(const SceneGraph& graph, ObjectId id) {
  auto path = PathToRoot(graph, id);
  if (!path.empty() && path.back() == id) path.pop_back();
  return path;
}

std::vector<ObjectId> LeafNodes(const SceneGraph& graph) {
  std::vector<ObjectId> leaves;
  for (const auto& entry : graph.nodes) {
    if (entry.second.children.empty()) leaves.push_back(entry.first);
  }
  SortIds(&leaves);
  return leaves;
}

std::vector<ObjectId> NodesAtDepth(const SceneGraph& graph, std::size_t depth) {
  std::vector<ObjectId> nodes;
  for (const auto& entry : graph.nodes) {
    if (entry.second.reachable && entry.second.depth == depth) nodes.push_back(entry.first);
  }
  SortIds(&nodes);
  return nodes;
}

std::vector<ObjectId> SiblingNodes(const SceneGraph& graph, ObjectId id) {
  auto found = graph.nodes.find(id);
  if (found == graph.nodes.end()) return {};
  auto parent = graph.nodes.find(found->second.parent);
  std::vector<ObjectId> siblings;
  if (parent == graph.nodes.end()) {
    siblings = graph.roots;
  } else {
    siblings = parent->second.children;
  }
  siblings.erase(std::remove(siblings.begin(), siblings.end(), id), siblings.end());
  SortIds(&siblings);
  return siblings;
}

std::vector<ObjectId> ReachableFrom(const SceneGraph& graph, ObjectId id) {
  std::vector<ObjectId> reachable;
  if (!GraphContainsNode(graph, id)) return reachable;
  std::deque<ObjectId> queue;
  std::unordered_set<ObjectId> seen;
  queue.push_back(id);
  seen.insert(id);
  while (!queue.empty()) {
    const ObjectId current = queue.front();
    queue.pop_front();
    reachable.push_back(current);
    auto found = graph.nodes.find(current);
    if (found == graph.nodes.end()) continue;
    for (ObjectId child : found->second.children) {
      if (seen.insert(child).second) queue.push_back(child);
    }
  }
  SortIds(&reachable);
  return reachable;
}

std::vector<std::vector<ObjectId>> ConnectedComponents(const SceneGraph& graph) {
  std::vector<std::vector<ObjectId>> components;
  std::unordered_set<ObjectId> seen;
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (ObjectId id : ids) {
    if (seen.find(id) != seen.end()) continue;
    std::vector<ObjectId> component;
    std::deque<ObjectId> queue;
    queue.push_back(id);
    seen.insert(id);
    while (!queue.empty()) {
      const ObjectId current = queue.front();
      queue.pop_front();
      component.push_back(current);
      auto found = graph.nodes.find(current);
      if (found == graph.nodes.end()) continue;
      std::vector<ObjectId> neighbors = found->second.children;
      if (found->second.parent != kInvalidObjectId) neighbors.push_back(found->second.parent);
      for (ObjectId neighbor : neighbors) {
        if (GraphContainsNode(graph, neighbor) && seen.insert(neighbor).second) {
          queue.push_back(neighbor);
        }
      }
    }
    SortIds(&component);
    components.push_back(component);
  }
  return components;
}

std::vector<GraphDepthBucket> GraphDepthHistogram(const SceneGraph& graph) {
  std::vector<GraphDepthBucket> buckets;
  for (const auto& entry : graph.nodes) {
    if (!entry.second.reachable) continue;
    auto found = std::find_if(buckets.begin(), buckets.end(), [&entry](const GraphDepthBucket& bucket) {
      return bucket.depth == entry.second.depth;
    });
    if (found == buckets.end()) {
      buckets.push_back({entry.second.depth, 1});
    } else {
      ++found->count;
    }
  }
  std::sort(buckets.begin(), buckets.end(), [](const GraphDepthBucket& left,
                                               const GraphDepthBucket& right) {
    return left.depth < right.depth;
  });
  return buckets;
}

std::size_t SubtreeSize(const SceneGraph& graph, ObjectId id) {
  if (!GraphContainsNode(graph, id)) return 0;
  return DescendantsOf(graph, id).size() + 1U;
}

ObjectId LowestCommonAncestor(const SceneGraph& graph, ObjectId left, ObjectId right) {
  const auto left_path = PathToRoot(graph, left);
  const auto right_path = PathToRoot(graph, right);
  ObjectId ancestor = kInvalidObjectId;
  const std::size_t count = std::min(left_path.size(), right_path.size());
  for (std::size_t i = 0; i < count; ++i) {
    if (left_path[i] != right_path[i]) break;
    ancestor = left_path[i];
  }
  return ancestor;
}

bool GraphContainsNode(const SceneGraph& graph, ObjectId id) {
  return graph.nodes.find(id) != graph.nodes.end();
}

bool GraphHasRoots(const SceneGraph& graph) {
  return !graph.roots.empty();
}

bool GraphHasCycles(const SceneGraph& graph) {
  return !graph.cycles.empty();
}

bool GraphHasUnreachableNodes(const SceneGraph& graph) {
  return !graph.unreachable.empty();
}

bool GraphIsForest(const SceneGraph& graph) {
  if (GraphHasCycles(graph)) return false;
  std::size_t edge_count = 0;
  for (const auto& entry : graph.nodes) {
    edge_count += entry.second.children.size();
  }
  if (graph.nodes.empty()) return true;
  return edge_count + graph.roots.size() == graph.nodes.size();
}

std::size_t GraphEdgeCount(const SceneGraph& graph) {
  std::size_t count = 0;
  for (const auto& entry : graph.nodes) {
    count += entry.second.children.size();
  }
  return count;
}

std::size_t GraphRootCount(const SceneGraph& graph) {
  return graph.roots.size();
}

Result<std::vector<ObjectId>> TopologicalNodeOrder(const SceneGraph& graph) {
  if (GraphHasCycles(graph)) {
    return Status::Error(ErrorCode::kMalformedData, "scene graph contains a parent cycle");
  }
  std::vector<ObjectId> order;
  for (const auto& step : TraverseDepthFirst(graph)) order.push_back(step.id);
  for (ObjectId id : graph.unreachable) order.push_back(id);
  SortIds(&order);
  return order;
}

SceneGraphSummary SummarizeSceneGraph(const SceneGraph& graph) {
  SceneGraphSummary summary;
  summary.node_count = graph.nodes.size();
  summary.root_count = graph.roots.size();
  summary.unreachable_count = graph.unreachable.size();
  summary.cycle_count = graph.cycles.size();
  for (const auto& entry : graph.nodes) {
    summary.edge_count += entry.second.children.size();
    if (entry.second.children.empty()) ++summary.leaf_count;
    summary.max_depth = std::max(summary.max_depth, entry.second.depth);
  }
  return summary;
}

std::string FormatSceneGraphSummary(const SceneGraphSummary& summary) {
  std::ostringstream out;
  out << "node_count=" << summary.node_count << "\n";
  out << "root_count=" << summary.root_count << "\n";
  out << "edge_count=" << summary.edge_count << "\n";
  out << "leaf_count=" << summary.leaf_count << "\n";
  out << "unreachable_count=" << summary.unreachable_count << "\n";
  out << "cycle_count=" << summary.cycle_count << "\n";
  out << "max_depth=" << summary.max_depth << "\n";
  return out.str();
}

std::string FormatSceneGraph(const SceneGraph& graph) {
  std::ostringstream out;
  out << FormatSceneGraphSummary(SummarizeSceneGraph(graph));
  out << "depth_histogram=";
  for (const auto& bucket : GraphDepthHistogram(graph)) {
    out << " " << bucket.depth << ":" << bucket.count;
  }
  out << "\n";
  out << "nodes=" << graph.nodes.size() << "\n";
  out << "roots=";
  for (ObjectId id : graph.roots) out << " " << id;
  out << "\n";
  out << "unreachable=";
  for (ObjectId id : graph.unreachable) out << " " << id;
  out << "\n";
  out << "cycles=" << graph.cycles.size() << "\n";
  for (const auto& step : TraverseDepthFirst(graph)) {
    out << step.depth << " " << step.path << "\n";
  }
  return out.str();
}

std::string WriteGraphDot(const SceneGraph& graph) {
  std::ostringstream out;
  out << "digraph scene {\n";
  out << "  rankdir=TB;\n";
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (ObjectId id : ids) {
    auto found = graph.nodes.find(id);
    const bool root = std::find(graph.roots.begin(), graph.roots.end(), id) != graph.roots.end();
    out << "  n" << id << " [label=\"" << id << "\"";
    if (root) out << ", shape=box";
    if (found != graph.nodes.end() && !found->second.reachable) out << ", style=dashed";
    out << "];\n";
  }
  for (ObjectId id : ids) {
    auto found = graph.nodes.find(id);
    if (found == graph.nodes.end()) continue;
    for (ObjectId child : found->second.children) {
      out << "  n" << id << " -> n" << child << ";\n";
    }
  }
  out << "}\n";
  return out.str();
}

std::string WriteGraphAdjacencyList(const SceneGraph& graph) {
  std::ostringstream out;
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (ObjectId id : ids) {
    out << id << ":";
    auto found = graph.nodes.find(id);
    if (found != graph.nodes.end()) {
      for (ObjectId child : found->second.children) out << " " << child;
    }
    out << "\n";
  }
  return out.str();
}

std::string WriteGraphCsv(const SceneGraph& graph) {
  std::ostringstream out;
  out << "id,parent,depth,reachable,children\n";
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (ObjectId id : ids) {
    auto found = graph.nodes.find(id);
    if (found == graph.nodes.end()) continue;
    out << id << "," << found->second.parent << "," << found->second.depth << ","
        << (found->second.reachable ? "true" : "false") << ",";
    for (std::size_t i = 0; i < found->second.children.size(); ++i) {
      if (i != 0) out << "|";
      out << found->second.children[i];
    }
    out << "\n";
  }
  return out.str();
}

std::string WriteGraphJson(const SceneGraph& graph) {
  std::ostringstream out;
  out << "{\"nodes\":[";
  std::vector<ObjectId> ids;
  for (const auto& entry : graph.nodes) ids.push_back(entry.first);
  SortIds(&ids);
  for (std::size_t index = 0; index < ids.size(); ++index) {
    auto found = graph.nodes.find(ids[index]);
    if (found == graph.nodes.end()) continue;
    if (index != 0) out << ",";
    out << "{\"id\":" << found->second.id << ",";
    out << "\"parent\":" << found->second.parent << ",";
    out << "\"depth\":" << found->second.depth << ",";
    out << "\"reachable\":" << (found->second.reachable ? "true" : "false") << ",";
    out << "\"children\":[";
    for (std::size_t child = 0; child < found->second.children.size(); ++child) {
      if (child != 0) out << ",";
      out << found->second.children[child];
    }
    out << "]}";
  }
  out << "],\"roots\":[";
  for (std::size_t i = 0; i < graph.roots.size(); ++i) {
    if (i != 0) out << ",";
    out << graph.roots[i];
  }
  out << "]}";
  return out.str();
}

std::string FormatGraphTraversal(const std::vector<GraphTraversalStep>& steps) {
  std::ostringstream out;
  for (const auto& step : steps) {
    out << step.depth << " " << step.id << " " << step.path << "\n";
  }
  return out.str();
}

std::vector<std::string> ValidateGraphConsistency(const SceneGraph& graph) {
  std::vector<std::string> issues;
  for (const auto& entry : graph.nodes) {
    const auto& node = entry.second;
    if (node.id != entry.first) {
      issues.push_back("node map key does not match node id");
    }
    if (node.parent != kInvalidObjectId && !GraphContainsNode(graph, node.parent)) {
      issues.push_back("node " + std::to_string(node.id) + " has missing parent " +
                       std::to_string(node.parent));
    }
    for (ObjectId child : node.children) {
      auto found = graph.nodes.find(child);
      if (found == graph.nodes.end()) {
        issues.push_back("node " + std::to_string(node.id) + " has missing child " +
                         std::to_string(child));
      } else if (found->second.parent != node.id) {
        issues.push_back("node " + std::to_string(node.id) + " child " +
                         std::to_string(child) + " does not point back to parent");
      }
    }
  }
  for (ObjectId root : graph.roots) {
    auto found = graph.nodes.find(root);
    if (found == graph.nodes.end()) {
      issues.push_back("root " + std::to_string(root) + " is not in node map");
    } else if (found->second.parent != kInvalidObjectId) {
      issues.push_back("root " + std::to_string(root) + " has a parent");
    }
  }
  return issues;
}

}  // namespace bse
