#include "nodeGraph.h"
#include "assetProvider.h"
#include "script.h"

#include <nlohmann/json.hpp>
#include <cstdint>
#include <vector>

namespace Nebula
{

  bool NodeGraphRuntime::loadFromJson(std::string_view json)
  {
    try
    {
      const auto parsed = nlohmann::json::parse(json);
      if (!parsed.is_object() || !parsed.contains("version"))
      {
        return false;
      }
      m_json = std::string(json);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool NodeGraphRuntime::loadFromFile(const IAssetProvider &assets, std::string_view logicalPath)
  {
    std::vector<uint8_t> bytes;
    if (!assets.readFile(logicalPath, bytes))
    {
      return false;
    }
    return loadFromJson(std::string(bytes.begin(), bytes.end()));
  }

  void NodeGraphRuntime::fireEvent(ScriptContext &ctx, std::string_view eventName)
  {
    if (m_json.empty())
    {
      return;
    }
    try
    {
      const auto root = nlohmann::json::parse(m_json);
      if (!root.contains("nodes") || !root.contains("links"))
      {
        return;
      }
      int startId = -1;
      for (const auto &node : root["nodes"])
      {
        if (node.value("type", "") == "Event" && node.value("name", "") == eventName)
        {
          startId = node.value("id", -1);
          break;
        }
      }
      if (startId < 0)
      {
        return;
      }

      constexpr int kMaxSteps = 32;
      int steps = 0;
      int current = startId;
      while (current >= 0 && steps < kMaxSteps)
      {
        ++steps;
        int next = -1;
        for (const auto &link : root["links"])
        {
          if (link.value("from", -1) == current)
          {
            next = link.value("to", -1);
            break;
          }
        }
        if (next < 0)
        {
          break;
        }
        for (const auto &node : root["nodes"])
        {
          if (node.value("id", -1) != next)
          {
            continue;
          }
          const std::string type = node.value("type", "");
          const std::string name = node.value("name", "");
          if (type == "Action" && name == "CompleteObjective")
          {
            if (ctx.log != nullptr)
            {
              ctx.log->info("[NodeGraph] Action CompleteObjective");
            }
          }
          current = next;
          break;
        }
      }
    }
    catch (...)
    {
      if (ctx.log != nullptr)
      {
        ctx.log->error("[NodeGraph] execution failed");
      }
    }
  }

}
