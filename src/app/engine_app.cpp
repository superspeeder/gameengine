//
// Created by andy on 4/30/2025.
//

#include "engine_app.hpp"

#include <glm/glm.hpp>

namespace app {
    glfw_lib::glfw_lib() {
        glfwInit();
    }

    glfw_lib::~glfw_lib() {
        glfwTerminate();
    }

    struct Vertex {
        glm::vec2 position;
        glm::vec4 color;
    };

    EngineApp::EngineApp() {
        m_RenderDevice   = std::make_shared<engine::RenderDevice>();
        m_Window         = std::make_shared<engine::Window>(m_RenderDevice);
        m_Swapchain      = std::make_shared<engine::Swapchain>(m_RenderDevice, m_Window);
        m_WindowRenderer = std::make_shared<engine::WindowRenderer>(m_RenderDevice, m_Swapchain);
        // m_Shader         = engine::Shader::create_linked(
        //     m_RenderDevice,
        //     {
        //         engine::ShaderInfo{
        //                     .stage     = vk::ShaderStageFlagBits::eVertex,
        //                     .nextStage = vk::ShaderStageFlagBits::eFragment,
        //                     .name      = "main",
        //                     .code      = engine::Shader::load_code("assets/shaders/main.vert.spv"),
        //                     .sil       = {}
        //         },
        //         engine::ShaderInfo{
        //                     .stage     = vk::ShaderStageFlagBits::eFragment,
        //                     .nextStage = vk::ShaderStageFlags(),
        //                     .name      = "main",
        //                     .code      = engine::Shader::load_code("assets/shaders/main.frag.spv"),
        //                     .sil       = {}
        //         },
        //     }
        // );

        m_Shader = engine::MaterialShader::create_shared(
            m_RenderDevice,
            {
                engine::MaterialShaderStage{
                    .path       = "assets/shaders/main.vert.spv",
                    .stage      = vk::ShaderStageFlagBits::eVertex,
                    .entryPoint = "main",
                    .sil        = {},
                },
                engine::MaterialShaderStage{
                    .path       = "assets/shaders/main.frag.spv",
                    .stage      = vk::ShaderStageFlagBits::eFragment,
                    .entryPoint = "main",
                    .sil        = {},
                },
            }
        );

        std::vector<Vertex> vertices = {
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
            {{0.0f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}},
        };

        m_VertexBuffer = engine::VertexBuffer::create(
            m_RenderDevice, engine::VertexBufferStorage::Static, vertices,
            {vk::VertexInputBindingDescription2EXT(0, sizeof(Vertex), vk::VertexInputRate::eVertex),
             {vk::VertexInputAttributeDescription2EXT(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)),
              vk::VertexInputAttributeDescription2EXT(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color))}}
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

                m_VertexBuffer->bindAndSetState(cmd, 0);

                cmd.draw(3, 1, 0, 0);
            });

            m_Swapchain->update();
        }
    }
} // namespace app
