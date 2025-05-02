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
        m_RenderDevice   = std::make_shared<engine::RenderDevice>();
        m_Window         = std::make_shared<engine::Window>(m_RenderDevice);
        m_Swapchain      = std::make_shared<engine::Swapchain>(m_RenderDevice, m_Window);
        m_WindowRenderer = std::make_shared<engine::WindowRenderer>(m_RenderDevice, m_Swapchain);
        m_Shader         = engine::Shader::create_linked(
            m_RenderDevice,
            {
                engine::ShaderInfo{
                            .stage     = vk::ShaderStageFlagBits::eVertex,
                            .nextStage = vk::ShaderStageFlagBits::eFragment,
                            .name      = "main",
                            .code      = engine::Shader::load_code("assets/shaders/main.vert.spv"),
                            .sil       = {}
                },
                engine::ShaderInfo{
                            .stage     = vk::ShaderStageFlagBits::eFragment,
                            .nextStage = vk::ShaderStageFlags(),
                            .name      = "main",
                            .code      = engine::Shader::load_code("assets/shaders/main.frag.spv"),
                            .sil       = {}
                },
            }
        );
    }

    EngineApp::~EngineApp() {
        m_RenderDevice->waitDeviceIdle();
    }

    void EngineApp::run() {
        while (!m_Window->shouldClose()) {
            glfwPollEvents();


            m_WindowRenderer->renderFrame([&](const vk::raii::CommandBuffer &cmd, const engine::SwapchainFrameInfo &frameInfo, uint32_t currentFrame) {
                frameInfo.setViewportAndScissor(cmd);
                engine::Shader::setGenericState(cmd);

                engine::Shader::bindNull(cmd);
                m_Shader->bindTo(cmd);

                cmd.draw(3, 1, 0, 0);
            });

            m_Swapchain->update();
        }
    }
} // namespace app
