#include "vulkan_GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Nebula::detail
{

    VulkanGraphicsContext::VulkanGraphicsContext(GLFWwindow *window) : m_window(window) {}

    void VulkanGraphicsContext::makeCurrent() {}

    void VulkanGraphicsContext::swap() {}

    bool VulkanGraphicsContext::isValid() const
    {
        return false;
    }

    void VulkanGraphicsContext::getFramebufferSize(int &width, int &height) const
    {
        if (!m_window)
        {
            width = 0;
            height = 0;
            return;
        }
        glfwGetFramebufferSize(m_window, &width, &height);
    }

}
