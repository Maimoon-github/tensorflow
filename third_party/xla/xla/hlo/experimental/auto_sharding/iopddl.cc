/*
Copyright 2024 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "xla/hlo/experimental/auto_sharding/iopddl.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

////////////////////////////////////////////////////////////////////////////////
/////////  Utilities for reading problems and evaluating solutions.    /////////
/////////  Contest participants do not need to modify this code.       /////////
////////////////////////////////////////////////////////////////////////////////

namespace iopddl {

bool Strategy::operator==(const Strategy& other) const {
  return cost == other.cost && usage == other.usage;
}

bool Node::operator==(const Node& other) const {
  return interval == other.interval && strategies == other.strategies;
}

bool Edge::operator==(const Edge& other) const {
  return nodes == other.nodes && strategies == other.strategies;
}

bool Problem::operator==(const Problem& other) const {
  return name == other.name && nodes == other.nodes && edges == other.edges &&
         usage_limit == other.usage_limit;
}

absl::StatusOr<TotalCost> Evaluate(const Problem& problem,
                                   const Solution& solution) {
  if (solution.size() != problem.nodes.size()) {
    return absl::InvalidArgumentError("Incorrect solution size");
  }
  TimeIdx max_time = 0;
  for (const Node& node : problem.nodes) {
    max_time = std::max(max_time, node.interval.second);
  }
  TotalCost cost = 0;
  std::vector<TotalUsage> total_usages(max_time);
  for (NodeIdx node_idx = 0; node_idx < problem.nodes.size(); ++node_idx) {
    const Node& node = problem.nodes[node_idx];
    const StrategyIdx strategy_idx = solution[node_idx];
    if (strategy_idx < 0 || strategy_idx >= (int64_t)node.strategies.size()) {
      return absl::OutOfRangeError("Invalid strategy index");
    }
    cost += node.strategies[strategy_idx].cost;
    for (TimeIdx t = node.interval.first; t < node.interval.second; ++t) {
      total_usages[t] += node.strategies[strategy_idx].usage;
    }
  }
  for (const Edge& edge : problem.edges) {
    StrategyIdx strategy_idx = 0;
    for (const NodeIdx node_idx : edge.nodes) {
      strategy_idx *= problem.nodes[node_idx].strategies.size();
      strategy_idx += solution[node_idx];
    }
    cost += edge.strategies[strategy_idx].cost;
  }
  if (problem.usage_limit) {
    for (const TotalUsage& total_usage : total_usages) {
      if (total_usage > *problem.usage_limit) {
        return absl::ResourceExhaustedError("Usage limit exceeded");
      }
    }
  }
  return cost;
}

}  // namespace iopddl
