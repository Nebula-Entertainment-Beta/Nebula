#pragma once

#include "graphicsContext.h"

struct GLFWwindow;

namespace Nebula::detail
{

    /** Placeholder until a Vulkan swapchain/device exists. */
    class VulkanGraphicsContext final : public graphicsContext
    {
    public:
        explicit VulkanGraphicsContext(GLFWwindow *window);

        void makeCurrent() override;
        void swap() override;
        bool isValid() const override;
        void getFramebufferSize(int &width, int &height) const override;

    private:
        GLFWwindow *m_window = nullptr;
    };

}
