//
// Created by andy on 4/30/2025.
//

#include "window.hpp"

namespace engine {
    Window::Window() {
        m_Window = glfwCreateWindow(640, 480, "Hello World!", nullptr, nullptr);
    }

    Window::~Window() {
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
} // namespace engine
