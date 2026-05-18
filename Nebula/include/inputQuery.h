#pragma once
#include "frameCommands.h" // FrameInput

namespace Nebula
{

  class IInputQuery
  {
  public:
    virtual ~IInputQuery() = default;
    virtual const FrameInput &frame() const = 0;
  };

  class FrameInputQuery final : public IInputQuery
  {
  public:
    explicit FrameInputQuery(const FrameInput &f) : m_frame(f) {}
    const FrameInput &frame() const override { return m_frame; }

  private:
    const FrameInput &m_frame;
  };

} // namespace Nebula