#include "ecs/entity.h"
#include "nodeGraph.h"
#include "environment.h"
#include "component.h"

#include <cassert>
#include <iostream>

int main()
{
  // Entity ordering
  Nebula::Entity a{1, 0};
  Nebula::Entity b{2, 0};
  assert(a < b);

  // Environment defaults
  Nebula::EnvironmentComponent env{};
  const Nebula::Vec4 clear = Nebula::environmentClearColor(env);
  assert(clear.w == 1.f);

  // Node graph load + fire (no crash)
  Nebula::NodeGraphRuntime graph;
  const bool ok = graph.loadFromJson(R"({
    "version": 1,
    "nodes": [{"id":1,"type":"Event","name":"OnObjectiveReady"},
              {"id":2,"type":"Action","name":"CompleteObjective"}],
    "links": [{"from":1,"to":2}]
  })");
  assert(ok);

  std::cout << "smoke_tests OK\n";
  return 0;
}
