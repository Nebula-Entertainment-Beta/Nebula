#pragma once

#include <string>
#include <string_view>

namespace Nebula
{

  class ScriptContext;

  /** Tiny runtime for versioned JSON graphs (event -> action bridge). */
  class NodeGraphRuntime
  {
  public:
    bool loadFromJson(std::string_view json);
    bool loadFromFile(const class IAssetProvider &assets, std::string_view logicalPath);
    void fireEvent(ScriptContext &ctx, std::string_view eventName);
    const std::string &sourceJson() const { return m_json; }

  private:
    std::string m_json;
  };

}
