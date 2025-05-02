//
// Created by andy on 4/30/2025.
//

#include "window.hpp"

#include <iostream>

namespace engine {
    Window::Window(const std::shared_ptr<RenderDevice> &render_system) : m_RenderDevice(render_system) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(640, 480, "Hello World!", nullptr, nullptr);

        VkSurfaceKHR surf;
        glfwCreateWindowSurface(*m_RenderDevice->instance(), m_Window, nullptr, &surf);

        m_Surface = {m_RenderDevice->instance(), surf};
    }

    Window::~Window() {
        m_Surface.clear();
        glfwDestroyWindow(m_Window);
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    glm::uvec2 Window::getSize() const {
        glm::ivec2 s;
        glfwGetFramebufferSize(m_Window, &s.x, &s.y);
        return s;
    }

    glm::dvec2 Window::getMousePosition() const {
        glm::dvec2 m;
        glfwGetCursorPos(m_Window, &m.x, &m.y);
        return m;
    }

    bool Window::getKey(const int key) const {
        return glfwGetKey(m_Window, key) == GLFW_PRESS;
    }

    bool Window::getButton(const int button) const {
        return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
    }

    void Window::close() const {
        glfwSetWindowShouldClose(m_Window, true);
    }

    vk::SurfaceCapabilitiesKHR Window::getSurfaceCapabilities() const {
        return m_RenderDevice->physicalDevice().getSurfaceCapabilitiesKHR(m_Surface);
    }

    std::vector<vk::SurfaceFormatKHR> Window::getSurfaceFormats() const {
        return m_RenderDevice->physicalDevice().getSurfaceFormatsKHR(m_Surface);
    }

    std::vector<vk::PresentModeKHR> Window::getPresentModes() const {
        return m_RenderDevice->physicalDevice().getSurfacePresentModesKHR(m_Surface);
    }

    vk::Extent2D Window::getSurfaceCompatibleExtent() const {
        auto size = getSize();
        auto caps = getSurfaceCapabilities();
        return {std::clamp(size.x, caps.minImageExtent.width, caps.maxImageExtent.width), std::clamp(size.y, caps.minImageExtent.height, caps.maxImageExtent.height)};
    }
} // namespace engine
