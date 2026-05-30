#pragma once
#include <string>

namespace Editor
{

  /** Holds the edit-scene JSON snapshot taken when entering play mode. */
  class EditorPlayMode
  {
  public:
    std::string &snapshot() { return m_editSceneSnapshot; }
    const std::string &snapshot() const { return m_editSceneSnapshot; }

  private:
    std::string m_editSceneSnapshot;
  };

}
