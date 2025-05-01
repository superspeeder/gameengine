//
// Created by andy on 4/30/2025.
//

#include "engine_app.hpp"

namespace app {
    glfw_lib::glfw_lib() {
        glfwInit();
    }

    glfw_lib::~glfw_lib() {
        glfwTerminate();
    }

    EngineApp::EngineApp() {
        m_RenderSystem = std::make_shared<engine::RenderSystem>();
        m_Window       = std::make_shared<engine::Window>(m_RenderSystem);
        m_Swapchain    = std::make_shared<engine::Swapchain>(m_RenderSystem, m_Window);
    }

    void EngineApp::run() {
        while (!m_Window->shouldClose()) {
            glfwPollEvents();
        }
    }
} // namespace app
